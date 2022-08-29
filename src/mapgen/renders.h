#pragma once
#include <map>

#include <GL/glew.h>

class Texture
{
public:
    Texture(unsigned int width, unsigned int height, GLenum format = GL_RGBA, float *data = 0);
    ~Texture();
    Texture(const Texture &other);                // Copy constructor
    Texture(Texture &&other) noexcept;            // Move constructor
    Texture &operator=(const Texture &other);     // Copy Assignment
    Texture &operator=(Texture &&other) noexcept; // Move assignment

    GLuint id() const { return m_id; }
    GLuint width() const { return m_width; }
    GLuint height() const { return m_height; }
    GLuint format() const { return m_format; }

    void resize(unsigned int width, unsigned int height);
    size_t numChannels() const;
    GLint internalFormat() const;

protected:
    GLint internalFormat(GLenum format) const;
    unsigned int m_width, m_height;
    GLenum m_format = GL_RGBA;
    GLuint m_id = 0;
};

typedef std::map<std::string, Texture *> RenderSet;
