#include <map>

#include "Texture.h"

Texture::Texture(unsigned int width, unsigned int height, GLenum format, float *data) : m_width(width), m_height(height), m_format(format)
{
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat(m_format), m_width, m_height, 0, m_format, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

Texture::~Texture()
{
    if (m_id)
    {
        glDeleteTextures(1, &m_id);
    }
}

// Copy constructor
Texture::Texture(const Texture &other)
{
    this->m_format = other.m_format;
    this->m_width = other.m_width;
    this->m_height = other.m_height;
    // Generate a new image
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    // Copy the image data
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat(m_format), m_width, m_height, 0, m_format, GL_FLOAT, 0);
    glCopyImageSubData(other.m_id, GL_TEXTURE_2D, 0, 0, 0, 0,
                       m_id, GL_TEXTURE_2D, 0, 0, 0, 0, m_width, m_height, 1);
}

// Move constructor
Texture::Texture(Texture &&other) noexcept
{
    this->m_id = other.m_id;
    this->m_format = other.m_format;
    this->m_width = other.m_width;
    this->m_height = other.m_height;
    other.m_id = 0;
}

// Copy Assignment
Texture &Texture::operator=(const Texture &other)
{
    this->m_format = other.m_format;
    this->m_width = other.m_width;
    this->m_height = other.m_height;
    // Generate a new image
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    // Copy the image data
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat(m_format), m_width, m_height, 0, m_format, GL_FLOAT, 0);
    glCopyImageSubData(other.m_id, GL_TEXTURE_2D, 0, 0, 0, 0,
                       m_id, GL_TEXTURE_2D, 0, 0, 0, 0, m_width, m_height, 1);
    return *this;
}

// Move assignment
Texture &Texture::operator=(Texture &&other) noexcept
{
    this->m_id = other.m_id;
    this->m_format = other.m_format;
    this->m_width = other.m_width;
    this->m_height = other.m_height;
    other.m_id = 0;
    return *this;
}

GLuint Texture::id() const { return m_id; }
unsigned int Texture::width() const { return m_width; }
unsigned int Texture::height() const { return m_height; }
glm::ivec2 Texture::imageSize() const { return {m_width, m_height}; }
GLuint Texture::format() const { return m_format; }

void Texture::resize(unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;
    glBindTexture(GL_TEXTURE_2D, m_id);
    // Data is intentionally not restructured
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat(m_format), m_width, m_height, 0, m_format, GL_FLOAT, 0);
}

size_t Texture::numChannels() const
{
    switch (m_format)
    {
    case GL_RGBA:
        return 4;
    case GL_RGB:
        return 3;
    case GL_RG:
        return 2;
    case GL_RED:
        return 1;
    default:
        throw "Unsupported format, only 32F formats currently supported";
    }
}

GLint Texture::internalFormat(GLenum format) const
{
    switch (format)
    {
    case GL_RGBA:
        return GL_RGBA32F;
    case GL_RGB:
        return GL_RGB32F;
    case GL_RG:
        return GL_RG32F;
    case GL_RED:
        return GL_R32F;
    default:
        throw "Unsupported format, only 32F formats currently supported";
    }
}

GLint Texture::internalFormat() const { return internalFormat(m_format); }

float *Texture::read() const
{
    float *buffer = new float[width() * height() * numChannels()];
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id());
    glGetTexImage(GL_TEXTURE_2D, 0, format(), GL_FLOAT, buffer);
    return buffer;
}
float *Texture::read(Channel channel) const
{
    float *buffer = new float[width() * height()];
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id());

    GLenum format;
    switch (channel)
    {
    case Channel_Red:
        format = GL_RED;
        break;
    case Channel_Green:
        format = GL_GREEN;
        break;
    case Channel_Blue:
        format = GL_BLUE;
        break;
    default:
        throw "Unknown channel";
    }
    glGetTexImage(GL_TEXTURE_2D, 0, format, GL_FLOAT, buffer);
    return buffer;
}
void Texture::write(float *pixels, unsigned int width, unsigned int height, unsigned int posx, unsigned int posy)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, posx, posy, width, height, GL_RGBA, GL_FLOAT, pixels);
}
void Texture::write(unsigned char *pixels, unsigned int width, unsigned int height, unsigned int posx, unsigned int posy)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, posx, posy, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}
