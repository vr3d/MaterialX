//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

class PyShaderGenerator : public mx::ShaderGenerator
{
  public:
    explicit PyShaderGenerator(mx::SyntaxPtr syntax) :
        mx::ShaderGenerator(syntax)
    {
    }

    const std::string& getLanguage() const override
    {
        PYBIND11_OVERLOAD_PURE(
            const std::string&,
            mx::ShaderGenerator,
            getLanguage
        );
    }

    const std::string& getTarget() const override
    {
        PYBIND11_OVERLOAD_PURE(
            const std::string&,
            mx::ShaderGenerator,
            getTarget
        );
    }

    mx::ShaderPtr generate(const std::string& name, mx::ElementPtr element, mx::GenContext& context) const override
    {
        PYBIND11_OVERLOAD_PURE(
            mx::ShaderPtr,
            mx::ShaderGenerator,
            generate,
            name,
            element,
            context
        );
    }

    void emitScopeBegin(mx::ShaderStage& stage, mx::ShaderStage::Brackets brackets) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitScopeBegin,
            stage,
            brackets
        );
    }

    void emitScopeEnd(mx::ShaderStage& stage, bool semicolon, bool newline) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitScopeEnd,
            stage,
            semicolon,
            newline
        );
    }

    void emitLineBegin(mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitLineBegin,
            stage
        );
    }

    void emitLineEnd(mx::ShaderStage& stage, bool semicolon) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitLineEnd,
            stage,
            semicolon
        );
    }

    void emitLineBreak(mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitLineBreak,
            stage
        );
    }

    void emitString(const std::string& str, mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitString,
            str,
            stage
        );
    }

    void emitLine(const std::string& str, mx::ShaderStage& stage, bool semicolon) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitLine,
            str,
            stage,
            semicolon
        );
    }

    void emitComment(const std::string& str, mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitComment,
            str,
            stage
        );
    }

    void emitBlock(const std::string& str, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitBlock,
            str,
            context,
            stage
        );
    }

    void emitInclude(const std::string& file, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitInclude,
            file,
            context,
            stage
        );
    }

    void emitFunctionDefinition(const mx::ShaderNode& node, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitFunctionDefinition,
            node,
            context,
            stage
        );
    }

    void emitFunctionCall(const mx::ShaderNode& node, mx::GenContext& context, mx::ShaderStage& stage, bool checkScope) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitFunctionCall,
            node,
            context,
            stage,
            checkScope
        );
    }

    void emitFunctionDefinitions(const mx::ShaderGraph& graph, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitFunctionDefinitions,
            graph,
            context,
            stage
        );
    }

    void emitFunctionCalls(const mx::ShaderGraph& graph, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitFunctionCalls,
            graph,
            context,
            stage
        );
    }

    void emitTypeDefinitions(mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitTypeDefinitions,
            context,
            stage
        );
    }

    void emitInput(const mx::ShaderInput* input, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitInput,
            input,
            context,
            stage
        );
    }

    void emitOutput(const mx::ShaderOutput* output, bool includeType, bool assignValue, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitOutput,
            output,
            includeType,
            assignValue,
            context,
            stage
        );
    }

    void emitVariableDeclarations(const mx::VariableBlock& block, const std::string& qualifier, const std::string& separator, mx::GenContext& context, mx::ShaderStage& stage, bool assignValue) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitVariableDeclarations,
            block,
            qualifier,
            separator,
            context,
            stage,
            assignValue
        );
    }

    void emitVariableDeclaration(const mx::ShaderPort* variable, const std::string& qualifier, mx::GenContext& context, mx::ShaderStage& stage, bool assignValue) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitVariableDeclaration,
            variable,
            qualifier,
            context,
            stage,
            assignValue
        );
    }

    std::string getUpstreamResult(const mx::ShaderInput* input, mx::GenContext& context) const override
    {
        PYBIND11_OVERLOAD(
            std::string,
            mx::ShaderGenerator,
            getUpstreamResult,
            input,
            context
        );
    }

    mx::ValuePtr remapEnumeration(const mx::ValueElementPtr& input, const mx::InterfaceElement& mappingElement, const mx::TypeDesc*& enumerationType) const override
    {
        PYBIND11_OVERLOAD(
            mx::ValuePtr,
            mx::ShaderGenerator,
            remapEnumeration,
            input,
            mappingElement,
            enumerationType
        );
    }

    mx::ValuePtr remapEnumeration(const std::string& inputName, const std::string& inputValue, const std::string& inputType, const mx::InterfaceElement& mappingElement, const mx::TypeDesc*& enumerationType) const override
    {
        PYBIND11_OVERLOAD(
            mx::ValuePtr,
            mx::ShaderGenerator,
            remapEnumeration,
            inputName,
            inputValue,
            inputType,
            mappingElement,
            enumerationType
        );
    }

  protected:
    mx::ShaderStagePtr createStage(const std::string& name, mx::Shader& shader) const override
    {
        PYBIND11_OVERLOAD(
            mx::ShaderStagePtr,
            mx::ShaderGenerator,
            createStage,
            name,
            shader
        );
    }

    mx::ShaderNodeImplPtr createSourceCodeImplementation(mx::ImplementationPtr impl) const override
    {
        PYBIND11_OVERLOAD(
            mx::ShaderNodeImplPtr,
            mx::ShaderGenerator,
            createSourceCodeImplementation,
            impl
        );
    }

    mx::ShaderNodeImplPtr createCompoundImplementation(mx::NodeGraphPtr impl) const override
    {
        PYBIND11_OVERLOAD(
            mx::ShaderNodeImplPtr,
            mx::ShaderGenerator,
            createCompoundImplementation,
            impl
        );
    }
};

void bindPyShaderGenerator(py::module& mod)
{
    py::class_<mx::ShaderGenerator, PyShaderGenerator, mx::ShaderGeneratorPtr>(mod, "ShaderGenerator")
        .def("getLanguage", &mx::ShaderGenerator::getLanguage)
        .def("getTarget", &mx::ShaderGenerator::getTarget)
        .def("generate", &mx::ShaderGenerator::generate)
        .def("setColorManagementSystem", &mx::ShaderGenerator::setColorManagementSystem)
        .def("getColorManagementSystem", &mx::ShaderGenerator::getColorManagementSystem);
}