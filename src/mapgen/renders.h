#pragma once
#include <map>

#include <GL/glew.h>

enum Layer
{
    LAYER_HEIGHTMAP
};

class Texture
{
protected:
    void LoadOnGPU();
    void DeleteLocalData();

public:
    unsigned int width, height;
    GLenum format = GL_RGBA;
    float *data = 0;
    GLuint ID = 0;

    Texture(unsigned int width, unsigned int height, GLenum format = GL_RGBA, float *data = 0);
    ~Texture();
    // TODO: Texture(const char* path) {} stbi load

    void Resize(unsigned int width, unsigned int height);
    bool IsOnGPU();
    void EnsureOnGPU();

    // TODO: Read/Write pixel methods, unload from GPU. This is to support CPU operators.
};

class RenderSet
{
protected:
    std::map<Layer, Texture *> layers;

public:
    RenderSet();

    void Reset();
    void AddLayer(Layer layer, Texture *texture);
    // bool HasLayer(Layer layer) {}
    Texture *GetLayer(Layer layer) const;
};
