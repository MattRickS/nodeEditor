#pragma once
#include <string>

#include <glm/glm.hpp>
#include <GL/glew.h>

std::string LoadFile(const char *filename);
GLuint CompileShader(const char *source, GLenum shaderType);
GLuint CompileProgram(size_t numShaders, GLuint *shaders);

class Shader
{
public:
    GLuint ID;

    Shader(const char *computeShader);
    Shader(const char *vertexPath, const char *fragmentPath);

    void use();
    // Utility uniform functions
    void setBool(const std::string &name, bool value) const;
    void setUInt(const std::string &name, unsigned int value) const;
    void setInt(const std::string &name, int value) const;
    void setInt2(const std::string &name, int x, int y) const;
    void setFloat(const std::string &name, float value) const;
    void setFloat2(const std::string &name, float x, float y) const;
    void setFloat3(const std::string &name, float x, float y, float z) const;
    void setFloat4(const std::string &name, float x, float y, float z, float w) const;
    void setVec2(const std::string &name, glm::vec2 vec) const;
    void setVec3(const std::string &name, glm::vec3 vec) const;
    void setVec4(const std::string &name, glm::vec4 vec) const;
    void setIVec2(const std::string &name, glm::ivec2 vec) const;
    void setMat4(const std::string &name, glm::mat4 &matrix) const;

private:
    GLuint getLocation(const std::string &name) const;
};
