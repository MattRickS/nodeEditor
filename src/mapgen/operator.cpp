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
    // TODO: format by layer type, eg, height is 1 channel
    outputs = std::vector<Texture>();
    for (const Layer layer : layers)
    {
        outputs.emplace_back(m_width, m_height, getLayerFormat(layer));
    }
}

void Operator::resize(unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;
    // Resize each output texture
    for (Texture &tex : outputs)
    {
        tex.resize(width, height);
    }
}
void Operator::preprocess(RenderSet *renders) {}

void Operator::PopulateRenderSet(RenderSet *renderSet)
{
    auto layers = outLayers();
    for (size_t i = 0; i < layers.size(); ++i)
    {
        (*renderSet)[layers[i]] = &outputs[i];
    }
}
