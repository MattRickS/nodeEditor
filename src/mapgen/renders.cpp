#include <map>

#include <GL/glew.h>

#include "renders.h"

Texture::Texture(unsigned int width, unsigned int height, GLenum format, float *data) : width(width), height(height), format(format)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat(format), width, height, 0, format, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

Texture::~Texture()
{
    if (ID)
    {
        glDeleteTextures(1, &ID);
    }
}

// Copy constructor
Texture::Texture(const Texture &other)
{
    // TODO
}

// Move constructor
Texture::Texture(Texture &&other) noexcept
{
    this->ID = other.ID;
    this->format = other.format;
    this->width = other.width;
    this->height = other.height;
    other.ID = 0;
}

// Copy Assignment
Texture &Texture::operator=(const Texture &other)
{
    // TODO
    return *this;
}

// Move assignment
Texture &Texture::operator=(Texture &&other) noexcept
{
    this->ID = other.ID;
    this->format = other.format;
    this->width = other.width;
    this->height = other.height;
    other.ID = 0;
    return *this;
}

void Texture::resize(unsigned int width, unsigned int height)
{
    this->width = width;
    this->height = height;
    glBindTexture(GL_TEXTURE_2D, ID);
    // TODO: Should probably just restructure the data so it's in the same pixel
    // positions, but either truncate or pad with black. For purposes of this tool,
    // deleting it should be fine.
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat(format), width, height, 0, format, GL_FLOAT, 0);
}

size_t Texture::numChannels() const
{
    switch (format)
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

GLint Texture::internalFormat() const { return internalFormat(format); }
