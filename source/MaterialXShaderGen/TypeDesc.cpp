#include <MaterialXShaderGen/TypeDesc.h>
#include <MaterialXShaderGen/Shader.h>

namespace MaterialX
{

namespace
{
    using TypeDescPtr = std::shared_ptr<TypeDesc>;
    using TypeDescMap = std::unordered_map<string, TypeDescPtr>;

    // Internal storage of the type descriptor pointers
    TypeDescMap& typeMap()
    {
        static TypeDescMap map;
        return map;
    }
}

TypeDesc::TypeDesc(const string& name, unsigned char basetype, unsigned char semantic, int size)
    : _name(name)
    , _basetype(basetype)
    , _semantic(semantic)
    , _size(size)
{
}

const TypeDesc* TypeDesc::registerType(const string& name, unsigned char basetype, unsigned char semantic, int size)
{
    TypeDescMap& map = typeMap();
    auto it = map.find(name);
    if (it != map.end())
    {
        throw ExceptionShaderGenError("A type with name '" + name + "' is already registered");
    }

    TypeDescPtr ptr = TypeDescPtr(new TypeDesc(name, basetype, semantic, size));
    map[name] = ptr;

    return ptr.get();
}

const TypeDesc* TypeDesc::get(const string& name)
{
    const TypeDescMap& map = typeMap();
    auto it = map.find(name);
    if (it == map.end())
    {
        throw ExceptionShaderGenError("No registered type with name '" + name + "' could be found");
    }
    return it->second.get();
}

namespace Type
{
    // Register all standard types and save their pointers
    // for quick access and type comparisons later.
    const TypeDesc* NONE               = TypeDesc::registerType("none", TypeDesc::BASETYPE_NONE);
    const TypeDesc* BOOLEAN            = TypeDesc::registerType("boolean", TypeDesc::BASETYPE_BOOLEAN);
    const TypeDesc* INTEGER            = TypeDesc::registerType("integer", TypeDesc::BASETYPE_INTEGER);
    const TypeDesc* FLOAT              = TypeDesc::registerType("float", TypeDesc::BASETYPE_FLOAT);
    const TypeDesc* VECTOR2            = TypeDesc::registerType("vector2", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMATIC_VECTOR, 2);
    const TypeDesc* VECTOR3            = TypeDesc::registerType("vector3", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMATIC_VECTOR, 3);
    const TypeDesc* VECTOR4            = TypeDesc::registerType("vector4", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMATIC_VECTOR, 4);
    const TypeDesc* COLOR2             = TypeDesc::registerType("color2", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMATIC_COLOR, 2);
    const TypeDesc* COLOR3             = TypeDesc::registerType("color3", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMATIC_COLOR, 3);
    const TypeDesc* COLOR4             = TypeDesc::registerType("color4", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMATIC_COLOR, 4);
    const TypeDesc* MATRIX33           = TypeDesc::registerType("matrix33", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMATIC_MATRIX, 9);
    const TypeDesc* MATRIX44           = TypeDesc::registerType("matrix44", TypeDesc::BASETYPE_FLOAT, TypeDesc::SEMATIC_MATRIX, 16);
    const TypeDesc* STRING             = TypeDesc::registerType("string", TypeDesc::BASETYPE_STRING);
    const TypeDesc* FILENAME           = TypeDesc::registerType("filename", TypeDesc::BASETYPE_STRING, TypeDesc::SEMATIC_FILENAME);
    const TypeDesc* BSDF               = TypeDesc::registerType("BSDF", TypeDesc::BASETYPE_NONE, TypeDesc::SEMATIC_CLOSURE);
    const TypeDesc* EDF                = TypeDesc::registerType("EDF", TypeDesc::BASETYPE_NONE, TypeDesc::SEMATIC_CLOSURE);
    const TypeDesc* VDF                = TypeDesc::registerType("VDF", TypeDesc::BASETYPE_NONE, TypeDesc::SEMATIC_CLOSURE);
    const TypeDesc* SURFACESHADER      = TypeDesc::registerType("surfaceshader", TypeDesc::BASETYPE_NONE, TypeDesc::SEMATIC_SHADER);
    const TypeDesc* VOLUMESHADER       = TypeDesc::registerType("volumeshader", TypeDesc::BASETYPE_NONE, TypeDesc::SEMATIC_SHADER);
    const TypeDesc* DISPLACEMENTSHADER = TypeDesc::registerType("displacementshader", TypeDesc::BASETYPE_NONE, TypeDesc::SEMATIC_SHADER);
    const TypeDesc* LIGHTSHADER        = TypeDesc::registerType("lightshader", TypeDesc::BASETYPE_NONE, TypeDesc::SEMATIC_SHADER);
}

}