#include <map>

#include <GL/glew.h>

#include "renders.h"

// =============================================================================
// Texture

void Texture::LoadOnGPU()
{
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // If the data's on the GPU it's likely being modified so do not keep a local
    // copy of the old data. Retrieve the data from the GPU if requested.
    DeleteLocalData();
}
void Texture::DeleteLocalData()
{
    if (data)
    {
        delete[] data;
    }
}

Texture::Texture(unsigned int width, unsigned int height, GLenum format, float *data) : width(width), height(height), format(format), data(data) {}
Texture::~Texture()
{
    if (ID)
    {
        glDeleteTextures(1, &ID);
    }
}

void Texture::Resize(unsigned int width, unsigned int height)
{
    this->width = width;
    this->height = height;
    glBindTexture(GL_TEXTURE_2D, ID);
    // TODO: Should probably just restructure the data so it's in the same pixel
    // positions, but either truncate or pad with black. For purposes of this tool,
    // deleting it should be fine.
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, 0);
    DeleteLocalData();
}
bool Texture::IsOnGPU() { return ID != 0; }
void Texture::EnsureOnGPU()
{
    if (!IsOnGPU())
    {
        glGenTextures(1, &ID);
        LoadOnGPU();
    }
}

// =============================================================================
// RenderSet

RenderSet::RenderSet() {}

void RenderSet::Reset() { layers.clear(); }
void RenderSet::AddLayer(Layer layer, Texture *texture)
{
    layers[layer] = texture;
}
// bool HasLayer(Layer layer) {}
Texture *RenderSet::GetLayer(Layer layer) const
{
    return layers.at(layer);
}
