#include <string>
#include <vector>

#include <GL/glew.h>

#include "../log.h"
#include "../nodegraph/Settings.h"
#include "PingPongOperator.h"

namespace Op
{
    PingPongOperator::PingPongOperator(const char *computeShader) : ComputeShaderOperator(computeShader) {}
    std::vector<Input> PingPongOperator::inputs() const
    {
        return {{}};
    }
    bool PingPongOperator::process(const std::vector<RenderSetOperator const *> &inputs, [[maybe_unused]] Settings const *settings, [[maybe_unused]] Settings const *sceneSettings)
    {
        LOG_DEBUG("Ping pong iteration: %d", m_iteration)
        m_shader.use();
        m_shader.setInt("_iteration", m_iteration);
        // TODO: ComputeShaderOperator should implement setting auto-load as func that ping pong calls here

        glm::ivec2 imageSize;
        if (m_iteration == 0)
        {
            Texture const *texture = inputs[0]->layer(DEFAULT_LAYER);
            if (!texture)
            {
                setError("Missing default layer for input texture");
                return false;
            }

            imageSize = texture->imageSize();
            Texture *pingTex = ensureOutputLayer(pingLayer, imageSize);
            ensureOutputLayer(pongLayer, imageSize);

            bindImage(0, texture, GL_READ_ONLY);
            bindImage(1, pingTex, GL_WRITE_ONLY);
        }
        else
        {
            Texture *pingTex = m_outputs.at(pingLayer);
            Texture *pongTex = m_outputs.at(pongLayer);

            if (m_iteration % 2 == 0)
            {
                // TODO: Make sure this isn't swapping the actual map layers...
                std::swap(pingTex, pongTex);
            }
            bindImage(0, pingTex, GL_READ_ONLY);
            bindImage(1, pongTex, GL_WRITE_ONLY);

            imageSize = pingTex->imageSize();
        }

        glDispatchCompute(ceil(imageSize.x / 8.0f), ceil(imageSize.y / 4.0f), 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glFinish();

        ++m_iteration;
        // Protective measure to prevent subclasses that forget to implement from running indefinitely.
        return true;
    }
    void PingPongOperator::reset()
    {
        ComputeShaderOperator::reset();
        m_iteration = 0;
    }

    int PingPongOperator::iteration() const { return m_iteration; }

    glm::ivec2 PingPongOperator::imageSize(const std::vector<RenderSetOperator const *> &inputs) const
    {
        Texture const *texture = inputs[0]->layer(DEFAULT_LAYER);
        if (!texture)
        {
            return {0, 0};
        }
        return texture->imageSize();
    }

    const std::string &PingPongOperator::currentOutputLayer() const
    {
        return (m_iteration % 2 == 0) ? pingLayer : pongLayer;
    }

    bool PingPongOperator::copyToLayer(const std::string &layer)
    {
        if (m_iteration == 0)
        {
            return false;
        }
        Texture *texture = m_outputs.at(currentOutputLayer());
        // XXX: Make sure this is a copy, not move...
        // Texture outputTexture{*texture};
        m_renderSet.emplace(layer, texture);
        return true;
    }
    void PingPongOperator::deletePingPongLayers()
    {
        m_outputs.erase(pingLayer);
        m_outputs.erase(pongLayer);
    }
}
