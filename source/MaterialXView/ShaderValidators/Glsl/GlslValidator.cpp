
#include <MaterialXView/External/GLew/glew.h>
#include <MaterialXView/Window/SimpleWindow.h>
#include <MaterialXView/OpenGL/GLUtilityContext.h>
#include <MaterialXView/ShaderValidators/Glsl/GlslValidator.h>

#include <iostream>
#include <algorithm>
#include <cmath>

namespace MaterialX
{
// OpenGL Constants
unsigned int GlslValidator::UNDEFINED_OPENGL_RESOURCE_ID = 0;
int GlslValidator::UNDEFINED_OPENGL_PROGRAM_LOCATION = -1;
int GlslValidator::ProgramInput::INVALID_OPENGL_TYPE = -1;

// View information
const float NEAR_PLANE = -100.0f;
const float FAR_PLANE = 100.0f;

GlslValidator::GlslValidator() :
    _programId(0),
    _colorTarget(0),
    _depthTarget(0),
    _frameBuffer(0),
    _frameBufferWidth(256),
    _frameBufferHeight(256),
    _indexBuffer(0),
    _indexBufferSize(0),
    _vertexArray(0),
    _dummyTexture(0),
    _hwShader(nullptr),
    _initialized(false)
{
    // Clear buffer ids to invalid identifier.
    _attributeBufferIds.resize(ATTRIBUTE_COUNT);
    std::fill(_attributeBufferIds.begin(), _attributeBufferIds.end(), UNDEFINED_OPENGL_RESOURCE_ID);
}

GlslValidator::~GlslValidator()
{
    deleteProgram();
    deleteTarget();

    GLUtilityContext* context = GLUtilityContext::get();
    if (context)
        GLUtilityContext::destroy();
}

void GlslValidator::setStage(const std::string& code, size_t stage)
{
    if (stage < HwShader::NUM_STAGES)
    {
        _stages[stage] = code;
    }

    // A stage change invalidates any cached parsed inputs
    clearInputLists();
    // For now a individual stage setting means to not use a HwShader.
    // If / when injection of stages is allowed then this should not longer be true
    _hwShader = nullptr;
}


void GlslValidator::setStages(const HwShaderPtr shader)
{
    if (!shader)
    {
        ShaderValidationErrorList errors;
        throw ExceptionShaderValidationError("Cannot set stages using null hardware shader.", errors);
    }

    // Clear out any old data
    clearStages();

    // Extract out the shader code per stage
    _hwShader = shader;
    for (size_t i = 0; i < HwShader::NUM_STAGES; i++)
    {
        _stages[i] = _hwShader->getSourceCode(i);
    }

    // A stage change invalidates any cached parsed inputs
    clearInputLists();
}

const std::string GlslValidator::getStage(size_t stage) const
{
    if (stage < HwShader::NUM_STAGES)
    {
        return _stages[stage];
    }
    return std::string();
}

void GlslValidator::clearStages()
{
    for (size_t i = 0; i < HwShader::NUM_STAGES; i++)
    {
        _stages[i].clear();
    }

    // Clearing stages invalidates any cached inputs
    clearInputLists();
}

bool GlslValidator::haveValidStages() const
{
    // Need at least a pixel shader stage and a vertex shader stage
    const std::string& vertexShaderSource = _stages[HwShader::VERTEX_STAGE];
    const std::string& fragmentShaderSource = _stages[HwShader::PIXEL_STAGE];

    return (vertexShaderSource.length() > 0 && fragmentShaderSource.length() > 0);
}


void GlslValidator::initialize()
{
    ShaderValidationErrorList errors;
    const std::string errorType("OpenGL utilities initialization.");

    if (!_initialized)
    {
        // Create window
        SimpleWindow window;
        const char* windowName = "Validator Window";
        bool created = window.create(const_cast<char *>(windowName),
                                    _frameBufferWidth, _frameBufferHeight,
                                    nullptr);
        if (!created)
        {
            errors.push_back("Failed to create window for testing.");
            throw ExceptionShaderValidationError(errorType, errors);
        }
        else
        {
            // Create offscreen context
            GLUtilityContext* context = GLUtilityContext::create(window.windowWrapper(), nullptr);
            if (!context)
            {
                errors.push_back("Failed to create OpenGL context for testing.");
                throw ExceptionShaderValidationError(errorType, errors);
            }
            else
            {
                if (context->makeCurrent())
                {
                    // Initialize glew
                    bool initializedFunctions = true;

                    glewInit();
                    if (!glewIsSupported("GL_VERSION_4_0"))
                    {
                        initializedFunctions = false;
                        errors.push_back("OpenGL version 4.0 not supported");
                        throw ExceptionShaderValidationError(errorType, errors);
                    }

                    if (initializedFunctions)
                    {
                        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
                        glClearStencil(0);

                        _initialized = true;
                    }
                }
            }
        }
    }
}

void GlslValidator::deleteTarget()
{
    if (_frameBuffer)
    {
        GLUtilityContext* context = GLUtilityContext::get();
        if (context && context->makeCurrent())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, UNDEFINED_OPENGL_RESOURCE_ID);
            glDeleteTextures(1, &_colorTarget);
            glDeleteTextures(1, &_depthTarget);
            glDeleteFramebuffers(1, &_frameBuffer);
        }
    }
}

bool GlslValidator::createTarget()
{
    ShaderValidationErrorList errors;
    const std::string errorType("OpenGL target creation failure.");

    GLUtilityContext* context = GLUtilityContext::get();
    if (!context)
    {
        errors.push_back("No valid OpenGL context to create target with.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
    if (!context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to create target with.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Only frame buffer only once
    if (_frameBuffer > 0)
    {
        return true;
    }

    // Set up frame buffer
    glGenFramebuffers(1, &_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);

    // Create an offscreen floating point color target and attach to the framebuffer
    _colorTarget = UNDEFINED_OPENGL_RESOURCE_ID;
    glGenTextures(1, &_colorTarget);
    glBindTexture(GL_TEXTURE_2D, _colorTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _frameBufferWidth, _frameBufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _colorTarget, 0);

    // Create floating point offscreen depth target
    _depthTarget = UNDEFINED_OPENGL_RESOURCE_ID;
    glGenTextures(1, &_depthTarget);
    glBindTexture(GL_TEXTURE_2D, _depthTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _frameBufferWidth, _frameBufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthTarget, 0);

    glBindTexture(GL_TEXTURE_2D, UNDEFINED_OPENGL_RESOURCE_ID);

    glDrawBuffer(GL_NONE);

    // Validate the framebuffer
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, UNDEFINED_OPENGL_RESOURCE_ID);
        glDeleteFramebuffers(1, &_frameBuffer);
        _frameBuffer = UNDEFINED_OPENGL_RESOURCE_ID;

        errors.push_back("Frame buffer object setup failed.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Unbind on cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, UNDEFINED_OPENGL_RESOURCE_ID);

    return true;
}

bool GlslValidator::bindTarget(bool bind)
{
    // Make sure we have a target to bind first
    createTarget();

    // Bind the frame buffer and route to color texture target
    if (bind)
    {
        if (!_frameBuffer)
        {
            ShaderValidationErrorList errors;
            errors.push_back("No framebuffer exists to bind.");
            throw ExceptionShaderValidationError("OpenGL target bind failure.", errors);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
        GLenum colorList[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, colorList);
    }
    // Unbind frame buffer and route nowhere.
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, UNDEFINED_OPENGL_RESOURCE_ID);
        glDrawBuffer(GL_NONE);
    }
    return true;
}

void GlslValidator::deleteProgram()
{
    if (_programId > 0)
    {
        GLUtilityContext* context = GLUtilityContext::get();
        if (context && context->makeCurrent())
        {
            glDeleteObjectARB(_programId);
        }
        _programId = UNDEFINED_OPENGL_RESOURCE_ID;
    }

    // Program deleted, so also clear cached input lists
    clearInputLists();
}

unsigned int GlslValidator::createProgram()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL program creation error.");

    GLUtilityContext* context = GLUtilityContext::get();
    if (!context)
    {
        errors.push_back("No valid OpenGL context to create program with.");
        throw ExceptionShaderValidationError(errorType, errors);

    }
    if (!context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to create program.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    deleteProgram();

    if (!haveValidStages())
    {
        errors.push_back("An invalid set of stages has been provided.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    GLint GLStatus = GL_FALSE;
    int GLInfoLogLength = 0;

    unsigned int stagesBuilt = 0;
    unsigned int desiredStages = 0;
    for (unsigned int i = 0; i < HwShader::NUM_STAGES; i++)
    {
        if (_stages[i].length())
            desiredStages++;
    }

    // Create vertex shader
    GLuint vertexShaderId = UNDEFINED_OPENGL_RESOURCE_ID;
    std::string &vertexShaderSource = _stages[HwShader::VERTEX_STAGE];
    if (vertexShaderSource.length())
    {
        vertexShaderId = glCreateShader(GL_VERTEX_SHADER);

        // Compile vertex shader
        const char* vertexChar = vertexShaderSource.c_str();
        glShaderSource(vertexShaderId, 1, &vertexChar, NULL);
        glCompileShader(vertexShaderId);

        // Check Vertex Shader
        glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &GLStatus);
        if (GLStatus == GL_FALSE)
        {
            errors.push_back("Error in compiling vertex shader:");
            glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &GLInfoLogLength);
            if (GLInfoLogLength > 0)
            {
                std::vector<char> vsErrorMessage(GLInfoLogLength + 1);
                glGetShaderInfoLog(vertexShaderId, GLInfoLogLength, NULL,
                    &vsErrorMessage[0]);
                errors.push_back(&vsErrorMessage[0]);
            }
        }
        else
        {
            stagesBuilt++;
        }
    }

    // Create fragment shader
    GLuint fragmentShaderId = UNDEFINED_OPENGL_RESOURCE_ID;
    std::string& fragmentShaderSource = _stages[HwShader::PIXEL_STAGE];
    if (fragmentShaderSource.length())
    {
        fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

        // Compile fragment shader
        const char *fragmentChar = fragmentShaderSource.c_str();
        glShaderSource(fragmentShaderId, 1, &fragmentChar, NULL);
        glCompileShader(fragmentShaderId);

        // Check fragment shader
        glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &GLStatus);
        if (GLStatus == GL_FALSE)
        {
            errors.push_back("Error in compiling fragment shader:");
            glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &GLInfoLogLength);
            if (GLInfoLogLength > 0)
            {
                std::vector<char> fsErrorMessage(GLInfoLogLength + 1);
                glGetShaderInfoLog(fragmentShaderId, GLInfoLogLength, NULL,
                    &fsErrorMessage[0]);
                errors.push_back(&fsErrorMessage[0]);
            }
        }
        else
        {
            stagesBuilt++;
        }
    }

    // Link stages to a programs
    if (stagesBuilt == desiredStages)
    {
        _programId = glCreateProgram();
        glAttachShader(_programId, vertexShaderId);
        glAttachShader(_programId, fragmentShaderId);
        glLinkProgram(_programId);

        // Check the program
        glGetProgramiv(_programId, GL_LINK_STATUS, &GLStatus);
        if (GLStatus == GL_FALSE)
        {
            errors.push_back("Error in linking program:");
            glGetProgramiv(_programId, GL_INFO_LOG_LENGTH, &GLInfoLogLength);
            if (GLInfoLogLength > 0)
            {
                std::vector<char> ProgramErrorMessage(GLInfoLogLength + 1);
                glGetProgramInfoLog(_programId, GLInfoLogLength, NULL,
                    &ProgramErrorMessage[0]);
                errors.push_back(&ProgramErrorMessage[0]);
            }
        }
    }

    // Cleanup
    if (vertexShaderId > 0)
    {
        if (_programId > 0)
        {
            glDetachShader(_programId, vertexShaderId);
        }
        glDeleteShader(vertexShaderId);
    }
    if (fragmentShaderId > 0)
    {
        if (_programId > 0)
        {
            glDetachShader(_programId, fragmentShaderId);
        }
        glDeleteShader(fragmentShaderId);
    }

    // If we encountered any errors while trying to create return list
    // of all errors. That is we collect all errors per stage plus any
    // errors during linking and throw one exception for them all so that
    // if there is a failure a complete set of issues is returned. We do
    // this after cleanup so keep GL state clean.
    if (errors.size())
    {
        throw ExceptionShaderValidationError(errorType, errors);
    }

    return _programId;
}

void GlslValidator::clearInputLists()
{
    _uniformList.clear();
    _attributeList.clear();
}

const GlslValidator::ProgramInputMap& GlslValidator::getUniformsList()
{
    return updateUniformsList();
}

const GlslValidator::ProgramInputMap& GlslValidator::getAttributesList()
{
    return updateAttributesList();
}

const GlslValidator::ProgramInputMap& GlslValidator::updateUniformsList()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL uniform parsing error.");

    if (_uniformList.size() > 0)
    {
        return _uniformList;
    }

    if (_programId <= 0)
    {
        errors.push_back("Cannot parse for uniforms without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Scan for textures
    int uniformCount = -1;
    int uniformSize = -1;
    GLenum uniformType = 0;
    int maxNameLength = 0;
    glGetProgramiv(_programId, GL_ACTIVE_UNIFORMS, &uniformCount);
    glGetProgramiv(_programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);
    char* uniformName = new char[maxNameLength];
    for (int i = 0; i < uniformCount; i++)
    {
        glGetActiveUniform(_programId, GLuint(i), maxNameLength, nullptr, &uniformSize, &uniformType, uniformName);
        GLint uniformLocation = glGetUniformLocation(_programId, uniformName);
        if (uniformLocation >= 0)
        {
            ProgramInputPtr inputPtr = std::make_shared<ProgramInput>(uniformLocation, uniformType, uniformSize);
            _uniformList[std::string(uniformName)] = inputPtr;
        }
    }
    delete[] uniformName;

    return _uniformList;
}

const GlslValidator::ProgramInputMap& GlslValidator::updateAttributesList()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL attribute parsing error.");

    if (_attributeList.size() > 0)
    {
        return _attributeList;
    }

    if (_programId <= 0)
    {
        errors.push_back("Cannot parse for attributes without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    GLint numAttributes = 0;
    GLint maxNameLength = 0;
    glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTES, &numAttributes);
    glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);
    char* attributeName = new char[maxNameLength];

    for (int i = 0; i < numAttributes; i++)
    {
        GLint attributeSize = 0;
        GLenum attributeType = 0;
        glGetActiveAttrib(_programId, GLuint(i), maxNameLength, nullptr, &attributeSize, &attributeType, attributeName);
        GLint attributeLocation = glGetAttribLocation(_programId, attributeName);
        if (attributeLocation >= 0)
        {
            ProgramInputPtr inputPtr = std::make_shared<ProgramInput>(attributeLocation, attributeType, attributeSize);
            _attributeList[std::string(attributeName)] = inputPtr;
            //std::cout << "Scanned attribute : " << attributeName << ". Location: " << attributeLocation << ". Type: " << attributeType << std::endl;
        }
    }
    delete[] attributeName;

    return _attributeList;
}

void GlslValidator::bindTimeAndFrame()
{
    ShaderValidationErrorList errors;

    if (_programId <= 0)
    {
        const std::string errorType("GLSL input binding error.");
        errors.push_back("Cannot bind time/frame without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    GLint location = UNDEFINED_OPENGL_PROGRAM_LOCATION;

    // Bind time
    auto programInput = _uniformList.find("u_time");
    if (programInput != _uniformList.end())
    {
        location = programInput->second->_location;
        if (location >= 0)
        {
            //std::cout << "Bind time. Location: " << location << std::endl;
            glUniform1f(location, 1.0f);
        }
    }

    // Bind frame
    programInput = _uniformList.find("u_frame");
    if (programInput != _uniformList.end())
    {
        location = programInput->second->_location;
        if (location >= 0)
        {
            //std::cout << "Bind frame. Location: " << location << std::endl;
            glUniform1f(location, 1.0f);
        }
    }
}


void GlslValidator::bindViewInformation()
{
    ShaderValidationErrorList errors;

    if (_programId <= 0)
    {
        const std::string errorType("GLSL input binding error.");
        errors.push_back("Cannot bind without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Pull information from HwShader
    if (_hwShader)
    {
    }

    GLint location = UNDEFINED_OPENGL_PROGRAM_LOCATION;

    // Set view direction and position
    auto programInput = _uniformList.find("u_viewPosition");
    if (programInput != _uniformList.end())
    {
        location = programInput->second->_location;
        if (location >= 0)
        {
            glUniform3f(location, NEAR_PLANE-1.0f, 0.0f, 0.0f);
        }
    }
    programInput = _uniformList.find("u_viewDirection");
    if (programInput != _uniformList.end())
    {
        location = programInput->second->_location;
        if (location >= 0)
        {
            glUniform3f(location, 1.0f, 0.0f, 0.0f);
        }
    }

    GLfloat mvm[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mvm);

    GLfloat pm[16];
    glGetFloatv(GL_PROJECTION_MATRIX, pm);

    // Set world related matrices
    //
    std::vector<std::string> worldMatrixVariables =
    {
        "u_worldMatrix",
        //"u_worldInverseMatrix",
        "u_worldTransposeMatrix",
        //"u_worldInverseTransposeMatrix"
    };
    for (auto worldMatrixVariable : worldMatrixVariables)
    {
        programInput = _uniformList.find(worldMatrixVariable);
        if (programInput != _uniformList.end())
        {
            location = programInput->second->_location;
            if (location >= 0)
            {
                bool transpose = (worldMatrixVariable.find("Transpose") != std::string::npos);
                glUniformMatrix4fv(location, 1, transpose, mvm);
            }
        }
    }

    // Bind projection matrices
    //
    std::vector<std::string> projectionMatrixVariables =
    {
        "u_projectionMatrix",
        //"u_projectionInverseMatrix",
        "u_projectionTransposeMatrix",
        //"u_projectionInverseTransposeMatrix",
    };
    for (auto projectionMatrixVariable : projectionMatrixVariables)
    {
        programInput = _uniformList.find(projectionMatrixVariable);
        if (programInput != _uniformList.end())
        {
            location = programInput->second->_location;
            if (location >= 0)
            {
                bool transpose = (projectionMatrixVariable.find("Transpose") != std::string::npos);
                glUniformMatrix4fv(location, 1, transpose, pm);
            }
        }
    }

    // Bind view related matrices
    std::vector<std::string> viewMatrixVariables =
    {
        //"u_viewMatrix",
        //"u_viewInverseMatrix",
        //"u_viewTransposeMatrix",
        //"u_viewInverseTransposeMatrix",
        "u_viewProjectionMatrix"
        //"u_worldViewProjectionMatrix"
    };
    for (auto viewMatrixVariable : viewMatrixVariables)
    {
        programInput = _uniformList.find(viewMatrixVariable);
        if (programInput != _uniformList.end())
        {
            location = programInput->second->_location;
            if (location >= 0)
            {
                glUniformMatrix4fv(location, 1, GL_FALSE, pm);
            }
        }
    }
    
    checkErrors();
}

void GlslValidator::findProgramInputs(const std::string& variable, 
                                         const ProgramInputMap& variableList,
                                         ProgramInputMap& foundList,
                                         bool exactMatch)
{
    foundList.clear();

    // Scan all attributes which match the attribute identifier completely or as a prefix
    //
    int ilocation = UNDEFINED_OPENGL_PROGRAM_LOCATION;
    auto programInput = variableList.find(variable);
    if (programInput != variableList.end())
    {
        ilocation = programInput->second->_location;
        if (ilocation >= 0)
        {
            foundList[variable] = programInput->second;
        }
    }
    else if (!exactMatch)
    {
        for (programInput = variableList.begin(); programInput != variableList.end(); programInput++)
        {
            const std::string& name = programInput->first;
            if (name.compare(0, variable.size(), variable) == 0)
            {
                ilocation = programInput->second->_location;
                if (ilocation >= 0)
                {
                    foundList[programInput->first] = programInput->second;
                }
            }
        }
    }
}

bool GlslValidator::bindAttribute(const GLfloat* bufferData,
                                    size_t bufferSize,
                                    const std::string& attributeId,
                                    const GlslValidator::AttributeIndex attributeIndex,
                                    unsigned int floatCount,
                                    bool exactMatch)
{
    ProgramInputMap foundList;
    findProgramInputs(attributeId, _attributeList, foundList, exactMatch);

    for (auto found : foundList)
    {
        int location = found.second->_location;
        //std::cout << "Bind attribute = " << attributeId << ". Location: " << location << std::endl;
        if (_attributeBufferIds[attributeIndex] < 1)
        {
            // Create a buffer based on attribute type.
            unsigned int buffer = UNDEFINED_OPENGL_RESOURCE_ID;
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, bufferSize, bufferData, GL_STATIC_DRAW);
            _attributeBufferIds[attributeIndex] = buffer;
        }

        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, floatCount, GL_FLOAT, GL_FALSE, 0, 0);
    }

    return (foundList.size() > 0);
}

void GlslValidator::bindGeometry()
{
    ShaderValidationErrorList errors;
    if (_programId <= 0)
    {
        const std::string errorType("GLSL matrix bind error.");
        errors.push_back("Cannot bind geometry without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Pull information from HwShader as needed
    if (_hwShader)
    {
        // To add: Pull any additional information required here
    }

    // Pull information from program directly
    {
        // Set up vertex arrays
        glGenVertexArrays(1, &_vertexArray);
        glBindVertexArray(_vertexArray);

        unsigned int indexData[] = { 0, 1, 2, 0, 2, 3 };
        _indexBufferSize = 6;
        glGenBuffers(1, &_indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

        // Bind positions
        const float border = 20.0f;
        const GLfloat positionData[] = { border, border, 0.0f,
            border, (float)(_frameBufferHeight)-border, 0.0f,
            (float)(_frameBufferWidth)-border, (float)(_frameBufferHeight)-border, 0.0f,
            (float)(_frameBufferWidth)-border, border, 0.0f };
        bindAttribute(positionData, sizeof(positionData), "i_position", POSITION3_ATTRIBUTE, 3, true);

        // Bind normals
        float normalData[] = { 0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f };
        bindAttribute(normalData, sizeof(normalData), "i_normal", NORMAL3_ATTRIBUTE, 3, true);

        // Bind tangents
        float tangentData[] = { 1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f };
        bindAttribute(tangentData, sizeof(tangentData), "i_tangent", TANGENT3_ATTRIBUTE, 3, true);

        // Bind bitangents
        float bitangentData[] = { 0.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f };
        bindAttribute(bitangentData, sizeof(bitangentData), "i_bitangent", BITANGENT3_ATTRIBUTE, 3, true);

        // Bind single set of colors for all locations found
        float colorData[] = { 1.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f };
        // Search for anything that starts with the prefix "i_color_"
        bindAttribute(colorData, sizeof(colorData), "i_color_", COLOR4_ATTRIBUTE, 4, false);

        // Bind single set of texture coords for all locations found
        GLfloat uvData[] = { 0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f };
        // Search for anything that starts with the prefix "i_texcoord_"
        bindAttribute(uvData, sizeof(uvData), "i_texcoord_", TEXCOORD2_ATTRIBUTE, 2, false);

        // Bind any named attribute information
        //
        std::string geomAttrPrefix("u_geomattr_");
        ProgramInputMap foundList;
        findProgramInputs(geomAttrPrefix, _uniformList, foundList, false);
        for (auto programInput : foundList)
        {
            // Only handle float1-4 types for now
            if (programInput.second->_type == GL_FLOAT)
            {
                GLfloat floatVal[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                int size = programInput.second->_size;
                if (size == 1)
                    glUniform1fv(programInput.second->_location, 1, floatVal);
                else if (size == 2)
                    glUniform2fv(programInput.second->_location, 1, floatVal);
                else if (size == 3)
                    glUniform3fv(programInput.second->_location, 1, floatVal);
                else if (size == 4)
                    glUniform4fv(programInput.second->_location, 1, floatVal);
            }
        }
    }

    checkErrors();
}

void GlslValidator::createDummyTexture(bool colored)
{
    if (_dummyTexture == UNDEFINED_OPENGL_RESOURCE_ID)
    {
        const unsigned int imageSize = 256;
        unsigned int middle = imageSize / 2;

        // Create a ramp texture for the dummy texture
        //
        GLubyte	 pixels[imageSize][imageSize][4];
        for (unsigned int i=0; i<imageSize; i++)
        {
            for (unsigned int j=0; j<imageSize; j++)
            {
                float fi = (float)i;
                float fj = (float)j;
                float dist = std::sqrt( std::pow((middle - fj), 2) + std::pow((middle - fi), 2) );
                dist /= imageSize;
                float mdist = (1.0f - dist);

                if (colored)
                {
                    pixels[i][j][0] = (GLubyte)(65 * dist);
                    pixels[i][j][1] = (GLubyte)(205 * dist);
                    pixels[i][j][2] = (GLubyte)(255 * dist);

                    pixels[i][j][0] += (GLubyte)(255 * mdist);
                    pixels[i][j][1] += (GLubyte)(147 * mdist);
                    pixels[i][j][2] += (GLubyte)(75 * mdist);
                }
                else
                {
                    pixels[i][j][0] = (GLubyte)(255 * mdist);
                    pixels[i][j][1] = (GLubyte)(255 * mdist);
                    pixels[i][j][2] = (GLubyte)(255 * mdist);
                }
                pixels[i][j][3] = (GLubyte)255;
            }
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &_dummyTexture);

        glBindTexture(GL_TEXTURE_2D, _dummyTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageSize, imageSize,
            0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        // Note: Must do this for default sampling to lookup properly.
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, UNDEFINED_OPENGL_RESOURCE_ID);
    }
}

void GlslValidator::unbindTextures()
{
    int textureUnit = 0;
    GLint maxImageUnits = -1;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxImageUnits);
    for (auto uniform : _uniformList)
    {
        GLenum uniformType = uniform.second->_type;
        GLint uniformLocation = uniform.second->_location;
        if (uniformLocation >= 0 &&
            uniformType >= GL_SAMPLER_1D && uniformType <= GL_SAMPLER_CUBE)
        {
            if (textureUnit >= maxImageUnits)
            {
                break;
            }

            // Unbind a texture to that unit
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, UNDEFINED_OPENGL_RESOURCE_ID);
            checkErrors();
            textureUnit++;
        }
    }
    glDeleteTextures(1, &_dummyTexture);
    _dummyTexture = UNDEFINED_OPENGL_RESOURCE_ID;

    checkErrors();
}

void GlslValidator::bindTextures()
{
    if (_programId <= 0)
    {
        const std::string errorType("GLSL matrix bind error.");
        ShaderValidationErrorList errors;
        errors.push_back("Cannot bind textures without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Pull information from program directly
    int textureUnit = 0;
    GLint maxImageUnits = -1;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxImageUnits);
    for (auto uniform : _uniformList)
    {
        GLenum uniformType = uniform.second->_type;
        GLint uniformLocation = uniform.second->_location;
        if (uniformLocation >= 0 &&
            uniformType >= GL_SAMPLER_1D && uniformType <= GL_SAMPLER_CUBE)
        {
            if (textureUnit >= maxImageUnits)
            {
                break;
            }

            // Map location to a texture unit incrementally
            glUniform1i(uniformLocation, textureUnit);
            // Bind a texture to that unit
            glActiveTexture(GL_TEXTURE0 + textureUnit);

            // Pull information from HwShader as needed
            bool textureBound = false;
            if (_hwShader)
            {
                // to add reader code
            }

            if (!textureBound)
            {
                createDummyTexture(true);
                glBindTexture(GL_TEXTURE_2D, _dummyTexture); // Bind a dummy texture
            }

            textureUnit++;
            //std::cout << "Bind sample:" << uniform.first << ". Location: " << uniformLocation << "Type: " << uniformType << std::endl;
        }
    }

    checkErrors();
}


void GlslValidator::unbindGeometry()
{
    // Cleanup attribute bindings
    //
    glBindVertexArray(0);
    int numberAttributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numberAttributes);
    for (int i = 0; i < numberAttributes; i++)
    {
        glDisableVertexAttribArray(i);
    }
    glBindBuffer(GL_ARRAY_BUFFER, UNDEFINED_OPENGL_RESOURCE_ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, UNDEFINED_OPENGL_RESOURCE_ID);

    // Clean up buffers
    //
    if (_indexBuffer > 0)
    {
        glDeleteBuffers(1, &_indexBuffer);
        _indexBuffer = UNDEFINED_OPENGL_RESOURCE_ID;
    }
    for (unsigned int i=0; i<ATTRIBUTE_COUNT; i++)
    {
        unsigned int bufferId = _attributeBufferIds[i];
        if (bufferId > 0)
        {
            glDeleteBuffers(1, &bufferId);
            _attributeBufferIds[i] = UNDEFINED_OPENGL_RESOURCE_ID;
        }
    }

    checkErrors();
}

void GlslValidator::bindInputs()
{
    if (_programId <= 0)
    {
        const std::string errorType("GLSL bind inputs error.");
        ShaderValidationErrorList errors;
        errors.push_back("Cannot bind inputs without a valid program");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Bind the program to use
    glUseProgram(_programId);
    checkErrors();

    // Parse for uniforms and attributes
    updateUniformsList();
    updateAttributesList();

    // Bind based on inputs found
    bindViewInformation();
    bindGeometry();
    bindTextures();
    bindTimeAndFrame();
}

void GlslValidator::render()
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL rendering error.");

    GLUtilityContext* context = GLUtilityContext::get();
    if (!context)
    {
        errors.push_back("No valid OpenGL context to render to.");
        throw ExceptionShaderValidationError(errorType, errors);
    }
    if (!context->makeCurrent())
    {
        errors.push_back("Cannot make OpenGL context current to render to.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Set up target
    bindTarget(true);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up viewing / projection matrices for an orthographic rendering
    glViewport(0, 0, _frameBufferWidth, _frameBufferHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, _frameBufferWidth, 0.0f, _frameBufferHeight, NEAR_PLANE, FAR_PLANE);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    try
    {
        // Bind program and input parameters
        if (_programId > 0)
        {
            // Check if we have any attributes to bind. If not then
            // there is nothing to draw
            GLint activeAttributeCount = 0;
            glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTES, &activeAttributeCount);

            if (activeAttributeCount <= 0)
            {
                errors.push_back("Program has no input vertex data.");
                throw ExceptionShaderValidationError(errorType, errors);
            }
            else
            {
                // Bind the program to use
                glUseProgram(_programId);
                checkErrors();

                bindInputs();

                glDrawElements(GL_TRIANGLES, (GLsizei)_indexBufferSize, GL_UNSIGNED_INT, (void*)0);
                checkErrors();

                // Unbind resources
                glUseProgram(0);
                unbindTextures();
                unbindGeometry();
            }
        }

        // Fallack draw some simple geometry
        else
        {
            glPushMatrix();
            glBegin(GL_QUADS);

            glTexCoord2f(0.0f, 1.0f);
            glNormal3f(1.0f, 0.0f, 0.0f);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex2f(0.0f, (float)_frameBufferHeight);

            glTexCoord2f(0.0f, 0.0f);
            glNormal3f(1.0f, 0.0, 0.0);
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex2f(0.0f, 0.0f);

            glTexCoord2f(1.0f, 0.0f);
            glNormal3f(1.0f, 0.0, 0.0);
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex2f((float)_frameBufferWidth, 0.0f);

            glTexCoord2f(1.0f, 1.0f);
            glNormal3f(1.0f, 0.0, 0.0);
            glColor3f(1.0f, 1.0f, 0.0f);
            glVertex2f((float)_frameBufferWidth, (float)_frameBufferHeight);

            glEnd();
            glPopMatrix();

            checkErrors();
        }
    }
    catch (ExceptionShaderValidationError e)
    {
        bindTarget(false);
        throw e;
    }

    // Unset target
    bindTarget(false);
}

void GlslValidator::save(std::string& fileName, const ImageHandlerPtr imageHandler)
{
    ShaderValidationErrorList errors;
    const std::string errorType("GLSL image save error.");

    if (!imageHandler)
    {
        errors.push_back("No image handler specified.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    size_t bufferSize = _frameBufferWidth * _frameBufferHeight * 4;
    float* buffer = new float[bufferSize];
    if (!buffer)
    {
        errors.push_back("Failed to read color buffer back.");
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Read back from the color texture.
    bindTarget(true);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, _frameBufferWidth, _frameBufferHeight, GL_RGBA, GL_FLOAT, buffer);
    bindTarget(false);
    try
    {
        checkErrors();
    }
    catch (ExceptionShaderValidationError e)
    {
        delete[] buffer;
        errors.push_back("Failed to read color buffer back.");
        errors.insert(std::end(errors), std::begin(e._errorLog), std::end(e._errorLog));
        throw ExceptionShaderValidationError(errorType, errors);
    }

    // Save using the handler
    bool saved = imageHandler->saveImage(fileName, "exr", _frameBufferWidth, _frameBufferHeight, 4, buffer);
    delete[] buffer;

    if (!saved)
    {
        errors.push_back("Faled to save to file:" + fileName);
        throw ExceptionShaderValidationError(errorType, errors);
    }
}

void GlslValidator::checkErrors()
{
    ShaderValidationErrorList errors;

    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        errors.push_back("OpenGL error: " + std::to_string(error));
    }
    if (errors.size())
    {
        throw ExceptionShaderValidationError("OpenGL context error.", errors);
    }
}

}