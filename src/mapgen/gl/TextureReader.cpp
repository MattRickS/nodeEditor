#include "TextureReader.h"

TextureReader::~TextureReader()
{
    deleteInternalBuffer();
}
Texture const *TextureReader::texture()
{
    return m_texture;
}
void TextureReader::setTexture(const Texture *texture)
{
    m_texture = texture;
}
bool TextureReader::ensureInternalBuffer()
{
    if (!m_texture)
    {
        return false;
    }
    size_t requiredSize = m_texture->width() * m_texture->height() * m_texture->numChannels();
    if (requiredSize > m_bufferSize)
    {
        deleteInternalBuffer();
        m_bufferSize = requiredSize;
        m_buffer = new float[m_bufferSize];
    }
    return true;
}
bool TextureReader::readDataFromGPU()
{
    if (!ensureInternalBuffer())
    {
        return false;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture->id());
    glGetTexImage(GL_TEXTURE_2D, 0, m_texture->format(), GL_FLOAT, m_buffer);
    return true;
}
glm::vec4 TextureReader::readPixel(int x, int y)
{
    if (!readDataFromGPU())
    {
        return {0, 0, 0, 0};
    }

    size_t index = (y * m_texture->width() + x) * m_texture->numChannels();
    glm::vec4 value;
    for (size_t i = 0; i < 4; ++i)
    {
        value[i] = i < m_texture->numChannels() ? m_buffer[index + i] : 0.0f;
    }
    return value;
}
void TextureReader::deleteInternalBuffer()
{
    if (m_buffer)
    {
        delete[] m_buffer;
    }
}
