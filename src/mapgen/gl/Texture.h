#pragma once
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "../constants.h"

class Texture
{
public:
    Texture(unsigned int width, unsigned int height, GLenum format = GL_RGBA, float *data = 0);
    ~Texture();
    Texture(const Texture &other);                // Copy constructor
    Texture(Texture &&other) noexcept;            // Move constructor
    Texture &operator=(const Texture &other);     // Copy Assignment
    Texture &operator=(Texture &&other) noexcept; // Move assignment

    GLuint id() const;
    unsigned int width() const;
    unsigned int height() const;
    glm::ivec2 imageSize() const;
    GLuint format() const;

    void resize(unsigned int width, unsigned int height);
    size_t numChannels() const;
    GLint internalFormat() const;

    // Reads a copy of the texture data. Memory is owned by the caller.
    float *read() const;
    float *read(Channel channel) const;
    void write(float *pixels, unsigned int width, unsigned int height, unsigned int posx = 0, unsigned int posy = 0);
    void write(unsigned char *pixels, unsigned int width, unsigned int height, unsigned int posx = 0, unsigned int posy = 0);

protected:
    GLint internalFormat(GLenum format) const;
    unsigned int m_width, m_height;
    GLenum m_format = GL_RGBA;
    GLuint m_id = 0;
};

typedef std::map<std::string, Texture *> RenderSet;
typedef std::map<const std::string, Texture const *> RenderSet_c;
