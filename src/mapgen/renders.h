#pragma once
#include <map>

#include <GL/glew.h>

class Texture
{
protected:
    GLint internalFormat(GLenum format) const;

public:
    unsigned int width, height;
    GLenum format = GL_RGBA;
    GLuint ID = 0;

    Texture(unsigned int width, unsigned int height, GLenum format = GL_RGBA, float *data = 0);
    ~Texture();
    Texture(const Texture &other);                // Copy constructor
    Texture(Texture &&other) noexcept;            // Move constructor
    Texture &operator=(const Texture &other);     // Copy Assignment
    Texture &operator=(Texture &&other) noexcept; // Move assignment
    // TODO: Texture(const char* path) {} stbi load

    void resize(unsigned int width, unsigned int height);
    size_t numChannels() const;
    GLint internalFormat() const;
};

typedef std::map<std::string, Texture *> RenderSet;
