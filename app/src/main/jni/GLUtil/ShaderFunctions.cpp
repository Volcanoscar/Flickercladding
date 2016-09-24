// ShaderFunctions.cpp

#include <stdio.h>
#include <string.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "GL_Includes.h"

#include "ShaderFunctions.h"
#include "Logging.h"
#ifdef __ANDROID__
#define LOG_INFO(...) LOGI(__VA_ARGS__)
#define LOG_ERROR(...) LOGE(__VA_ARGS__)
#define OutputPrint(...) LOGI(__VA_ARGS__)
#else
#include "Logger.h"
#include "DebugOutput.h"
#endif

#include "g_shaders.h"

// Convenience wrapper for setting uniform variables
GLint getUniLoc(const GLuint program, const GLchar *name)
{
    GLint loc;
    loc = glGetUniformLocation(program, name);
    if (loc == -1)
        LOG_ERROR("No such uniform named \"%s\"", name);
    return loc;
}

// Got this from http://www.lighthouse3d.com/opengl/glsl/index.php?oglinfo
// it prints out shader info (debugging!)
void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
    if (infologLength > 1)
    {
        infoLog = new char[infologLength];
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        LOG_ERROR("%s", infoLog);
        delete [] infoLog;
    }
}

// Got this from http://www.lighthouse3d.com/opengl/glsl/index.php?oglinfo
// it prints out shader info (debugging!)
void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 1)
    {
        int charsWritten  = 0;
        char *infoLog = NULL;
        infoLog = new char[infologLength];
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        LOG_ERROR("%s", infoLog);
        delete [] infoLog;
    }
}

// Retrieve shader source from a std::map of hard-coded shaders generated by
// shader_hardcoder.py to the file shaderlist.h.
const std::string GetShaderSourceFromTable(const char* filename)
{
    if (g_shaderMap.empty())
    {
        initShaderList();
    }

    if (g_shaderMap.count(filename) > 0)
    {
        return g_shaderMap[filename];
    }
    return "";
}

// Do not attempt to load shaders from file on device.
#if defined(_WIN32_WCE) || defined (__ANDROID__)
#  define LOAD_SHADERS_FROM_FILESYSTEM 0
#else
#  define LOAD_SHADERS_FROM_FILESYSTEM 1
#endif

#if LOAD_SHADERS_FROM_FILESYSTEM
// Load a string of shader source from the given filename in the data/ directory
// and return a copy of it.
const std::string GetShaderSourceFromFile(const char* filename)
{
    const std::string shaderHomedir = "../shaders/";
    const std::string shaderPath = shaderHomedir + filename;

    std::ifstream t(shaderPath.c_str());
    std::stringstream shaderSource;
    shaderSource << t.rdbuf();
    return shaderSource.str();
}
#endif

// Return shader source from filename, if it can be retrieved.
// If not, fall back to the hard-coded array in our global std::map.
const std::string GetShaderSource(const char* filename)
{
#if LOAD_SHADERS_FROM_FILESYSTEM
    const std::string fileSrc = GetShaderSourceFromFile(filename);
    if (fileSrc.empty() == false)
    {
        return fileSrc;
    }
#endif
    return GetShaderSourceFromTable(filename);
}

// http://stackoverflow.com/questions/1494399/how-do-i-search-find-and-replace-in-a-standard-string
void myReplace(std::string& str,
               const std::string& oldStr,
               const std::string& newStr)
{
    std::string::size_type pos = 0u;
    while((pos = str.find(oldStr, pos)) != std::string::npos)
    {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

// Once source is obtained from either file or hard-coded map, compile the
// shader, release the string memory and return the ID.
GLuint loadShaderFile(const char* filename, const unsigned long Type)
{
    std::string shaderSource = GetShaderSource(filename);
#ifdef _MACOS
    myReplace(shaderSource, "#version 310 es", "#version 330");
#endif
    
    if (shaderSource.empty())
        return 0;
    GLint length = shaderSource.length();

    //LOG_INFO("Shader source(%d): %s", length, shaderSource.c_str());

    GLuint shaderId = glCreateShader(Type);
    const GLchar* pSS = &shaderSource[0];
    glShaderSource(shaderId, 1, &pSS, &length);
    glCompileShader(shaderId);

    return shaderId;
}

// Append any applicable suffixes to the name given and attempt to find
// vertex, fragment and (optionally) geometry shader source.
GLuint makeShaderByName(const char* name)
{
    if (!name)
        return 0;

    std::string vs(name);
    std::string fs(name);
    vs += ".vert";
    fs += ".frag";

    LOG_INFO("Create shader: [%s] ...", name);

    GLint compileStatus = GL_FALSE;

    /// Vertex shader
    GLuint vertSrc = loadShaderFile(vs.c_str(), GL_VERTEX_SHADER);
    compileStatus = GL_FALSE;
    glGetShaderiv(vertSrc, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        printShaderInfoLog(vertSrc);
    }

    /// Fragment shader
    GLuint fragSrc = loadShaderFile(fs.c_str(), GL_FRAGMENT_SHADER);
    compileStatus = GL_FALSE;
    glGetShaderiv(fragSrc, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        printShaderInfoLog(fragSrc);
    }

    GLuint program = glCreateProgram();

    glCompileShader(vertSrc);
    glCompileShader(fragSrc);

    glAttachShader(program, vertSrc);
    glAttachShader(program, fragSrc);

    // Will be deleted when program is.
    glDeleteShader(vertSrc);
    glDeleteShader(fragSrc);

    glLinkProgram(program);

    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_TRUE)
    {
        LOG_INFO("  success.");
    }
    else
    {
        LOG_ERROR("  Link failed: ");
        printProgramInfoLog(program);
    }

    glUseProgram(0);
    return program;
}

GLuint makeShaderFromSource(
    const char* vert,
    const char* frag)
{
    const GLuint vertSrc = loadShaderFile(vert, GL_VERTEX_SHADER);
    printShaderInfoLog(vertSrc);

    const GLuint fragSrc = loadShaderFile(frag, GL_FRAGMENT_SHADER);
    printShaderInfoLog(fragSrc);

    // Vertex and fragment shaders are required
    if ((vertSrc == 0) || (fragSrc == 0))
    {
        LOG_ERROR("  SHADER NOT COMPILED - source not found.");
        return 0;
    }

    const GLuint program = glCreateProgram();

    glCompileShader(vertSrc);
    glCompileShader(fragSrc);

    GLint success = 0;
    glGetShaderiv(vertSrc, GL_COMPILE_STATUS, &success);
    //assert(success == GL_TRUE);
    glGetShaderiv(fragSrc, GL_COMPILE_STATUS, &success);
    //assert(success == GL_TRUE);

    glAttachShader(program, vertSrc);
    glAttachShader(program, fragSrc);

    // Will be deleted when program is.
    glDeleteShader(vertSrc);
    glDeleteShader(fragSrc);

    glLinkProgram(program);
    printProgramInfoLog(program);

    glUseProgram(0);
    return program;
}
