#include <fstream>
#include <sstream>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "util.h"

std::string loadFile(const char *filename)
{
    std::string code;
    std::ifstream stream;
    stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        stream.open(filename);
        std::stringstream sstr;
        sstr << stream.rdbuf();
        stream.close();
        code = sstr.str();
    }
    catch (std::ifstream::failure const &)
    {
        LOG_ERROR("Unable to open %s", filename);
    }

    return code;
}

GLuint compileShader(const char *source, GLenum shaderType)
{
    GLuint shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &source, NULL);
    glCompileShader(shaderID);

    GLint success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        LOG_ERROR("Error compiling shader: %s", infoLog);
        return 0;
    }

    return shaderID;
}

GLuint compileProgram(size_t numShaders, GLuint *shaders)
{
    GLuint programID = glCreateProgram();
    for (size_t i = 0; i < numShaders; ++i)
    {
        glAttachShader(programID, shaders[i]);
    }
    glLinkProgram(programID);

    GLint success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        LOG_ERROR("Error linking program: %s", infoLog);
        return 0;
    }

    return programID;
}

Shader::Shader(const char *computeShader)
{
    std::string source = loadFile(computeShader);
    GLuint shader = compileShader(source.c_str(), GL_COMPUTE_SHADER);

    ID = compileProgram(1, &shader);

    glDeleteShader(shader);
}

Shader::Shader(const char *vertexPath, const char *fragmentPath)
{
    std::string vertexSource = loadFile(vertexPath);
    std::string fragmentSource = loadFile(fragmentPath);

    GLuint vertexShader = compileShader(vertexSource.c_str(), GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource.c_str(), GL_FRAGMENT_SHADER);

    GLuint shaders[2] = {vertexShader, fragmentShader};
    ID = compileProgram(2, shaders);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::use()
{
    glUseProgram(ID);
}

// Utility uniform functions
GLuint Shader::getLocation(const std::string &name) const
{
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        LOG_WARNING("Shader uniform not found: %s", name.c_str());
    }
    return location;
}

void Shader::setBool(const std::string &name, bool value) const
{
    GLuint location = getLocation(name);
    glUniform1i(location, (int)value);
}

void Shader::setUInt(const std::string &name, unsigned int value) const
{
    GLuint location = getLocation(name);
    glUniform1ui(location, value);
}

void Shader::setInt(const std::string &name, int value) const
{
    GLuint location = getLocation(name);
    glUniform1i(location, value);
}

void Shader::setInt2(const std::string &name, int x, int y) const
{
    GLuint location = getLocation(name);
    glUniform2i(location, x, y);
}

void Shader::setFloat(const std::string &name, float value) const
{
    GLuint location = getLocation(name);
    glUniform1f(location, value);
}

void Shader::setFloat2(const std::string &name, float x, float y) const
{
    GLuint location = getLocation(name);
    glUniform2f(location, x, y);
}

void Shader::setFloat3(const std::string &name, float x, float y, float z) const
{
    GLuint location = getLocation(name);
    glUniform3f(location, x, y, z);
}

void Shader::setFloat4(const std::string &name, float x, float y, float z, float w) const
{
    GLuint location = getLocation(name);
    glUniform4f(location, x, y, z, w);
}

void Shader::setVec2(const std::string &name, glm::vec2 vec) const
{
    GLuint location = getLocation(name);
    glUniform2f(location, vec.x, vec.y);
}

void Shader::setVec3(const std::string &name, glm::vec3 vec) const
{
    GLuint location = getLocation(name);
    glUniform3f(location, vec.x, vec.y, vec.z);
}

void Shader::setVec4(const std::string &name, glm::vec4 vec) const
{
    GLuint location = getLocation(name);
    glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
}

void Shader::setIVec2(const std::string &name, glm::ivec2 vec) const
{
    GLuint location = getLocation(name);
    glUniform2i(location, vec.x, vec.y);
}

void Shader::setMat4(const std::string &name, glm::mat4 &matrix) const
{
    GLuint location = getLocation(name);
    // Location, Number of Matrices, Transpose?, matrices
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}
