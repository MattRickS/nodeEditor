#pragma once
#include <map>

#include <GL/glew.h>

enum Layer
{
    LAYER_HEIGHTMAP,
    LAYER_MAX
};

const char *getLayerName(Layer layer);

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

typedef std::map<Layer, Texture *> RenderSet;
