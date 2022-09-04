#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "Texture.h"

class TextureReader
{
public:
    ~TextureReader();

    const Texture *texture();
    void setTexture(const Texture *texture);
    glm::vec4 readPixel(int x, int y);

protected:
    const Texture *m_texture = nullptr;
    float *m_buffer = nullptr;
    size_t m_bufferSize = 0;

    bool ensureInternalBuffer();
    bool readDataFromGPU();
    void deleteInternalBuffer();
};
