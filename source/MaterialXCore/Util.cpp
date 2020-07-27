//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Util.h>

#include <cctype>

namespace MaterialX
{

const string EMPTY_STRING;

namespace {

const string LIBRARY_VERSION_STRING = std::to_string(MATERIALX_MAJOR_VERSION) + "." +
                                      std::to_string(MATERIALX_MINOR_VERSION) + "." +
                                      std::to_string(MATERIALX_BUILD_VERSION);

const std::tuple<int, int, int> LIBRARY_VERSION_TUPLE(MATERIALX_MAJOR_VERSION,
                                                      MATERIALX_MINOR_VERSION,
                                                      MATERIALX_BUILD_VERSION);

bool invalidNameChar(char c)
{
     return !isalnum(c) && c != '_' && c != ':';
}

} // anonymous namespace

//
// Utility methods
//

string getVersionString()
{
    return LIBRARY_VERSION_STRING;
}

std::tuple<int, int, int> getVersionIntegers()
{
    return LIBRARY_VERSION_TUPLE;
}

string makeVersionString(int majorVersion, int minorVersion)
{
    return std::to_string(majorVersion) + "." + std::to_string(minorVersion);
}

string createValidName(string name, char replaceChar)
{
    std::replace_if(name.begin(), name.end(), invalidNameChar, replaceChar);
    return name;
}

bool isValidName(const string& name)
{
    auto it = std::find_if(name.begin(), name.end(), invalidNameChar);
    return it == name.end();
}

string incrementName(const string& name)
{
    size_t split = name.length();
    while (split > 0)
    {
        if (!isdigit(name[split - 1]))
            break;
        split--;
    }

    if (split < name.length())
    {
        string prefix = name.substr(0, split);
        string suffix = name.substr(split, name.length());
        return prefix + std::to_string(std::stoi(suffix) + 1);
    }
    return name + "2";
}

StringVec splitString(const string& str, const string& sep)
{
    StringVec split;

    string::size_type lastPos = str.find_first_not_of(sep, 0);
    string::size_type pos = str.find_first_of(sep, lastPos);

    while (pos != string::npos || lastPos != string::npos)
    {
        split.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(sep, pos);
        pos = str.find_first_of(sep, lastPos);
    }

    return split;
}

string replaceSubstrings(string str, const StringMap& stringMap)
{
    for (const auto& pair : stringMap)
    {
        if (pair.first.empty())
            continue;

        size_t pos = 0;
        while ((pos = str.find(pair.first, pos)) != string::npos)
        {
             str.replace(pos, pair.first.length(), pair.second);
             pos += pair.second.length();
        }
    }
    return str;
}

string stringToLower(string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c)
    {
        return (char) std::tolower(c);
    });
    return str;
}

bool stringEndsWith(const string& str, const string& suffix)
{
    if (str.length() >= suffix.length())
    {
        return !str.compare(str.length() - suffix.length(), suffix.length(), suffix);
    }
    return false;
}

} // namespace MaterialX
