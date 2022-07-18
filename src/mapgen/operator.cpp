#include <iostream>
#include <vector>

#include <GL/glew.h>

#include "operator.h"

void Operator::init(unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;

    // Generate an output texture of the right size for each
    auto layers = outLayers();
    outputs = std::vector<Texture>(layers.size(), {m_width, m_height});

    // TODO: FBO/GPU setup is only required for shader operators, move to subclass
    for (Texture &texture : outputs)
    {
        texture.EnsureOnGPU();
    }

    // Generate the FBO with textures bound in order
    glGenFramebuffers(1, &FBO);
    // TODO: Binding only the draw buffer here causes it to fail. More research into this required.
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    std::vector<GLenum> drawBuffers(outputs.size());
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, outputs[i].ID, 0);
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    glDrawBuffers(drawBuffers.size(), drawBuffers.data());
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Failed to generate texture buffer" << std::endl;
    }
}

void Operator::resize(unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;
    // Resize each output texture
    for (Texture &tex : outputs)
    {
        tex.Resize(width, height);
    }
}

void Operator::PopulateRenderSet(RenderSet *renderSet)
{
    auto layers = outLayers();
    for (size_t i = 0; i < layers.size(); ++i)
    {
        (*renderSet)[layers[i]] = &outputs[i];
    }
}
