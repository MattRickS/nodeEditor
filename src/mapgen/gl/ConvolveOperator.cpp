#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../gl/ConvolveKernel.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"
#include "../constants.h"
#include "../log.h"

#include "ConvolveOperator.h"

namespace Op
{
    ConvolveOperator::ConvolveOperator() : ComputeShaderOperator("src/mapgen/operators/Convolve.glsl")
    {
        glGenBuffers(1, &m_ssbo);
    }
    ConvolveOperator::~ConvolveOperator()
    {
        glDeleteBuffers(1, &m_ssbo);
    }
    std::vector<Input> ConvolveOperator::inputs() const
    {
        return {{}};
    }
    void ConvolveOperator::registerSettings(Settings *const settings) const
    {
        settings->registerInt("channelMask", ChannelMask_RGB, ChannelMask_None, ChannelMask_Alpha, SettingHint_ChannelMask);
    }
    bool ConvolveOperator::process(const std::vector<RenderSetOperator const *> &inputs,
                                   Settings const *settings,
                                   [[maybe_unused]] Settings const *sceneSettings)
    {
        Texture const *inputTexture = inputs[0]->layer(DEFAULT_LAYER);
        if (!inputTexture)
        {
            setError("Missing default layer for input texture");
            return false;
        }
        m_shader.use();

        if (!populateKernel(&m_kernel, inputs, settings, sceneSettings))
        {
            setError("Failed to load kernel");
            return false;
        }
        LOG_DEBUG("Loading kernel (%u, %u)", m_kernel.width(), m_kernel.height())
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_ssbo);
        m_kernel.loadBuffer(GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW);

        Texture *outputTexture = ensureOutputLayer(DEFAULT_LAYER, {inputTexture->width(), inputTexture->height()});

        bindImage(0, inputTexture, GL_READ_ONLY);
        bindImage(1, outputTexture, GL_WRITE_ONLY);

        glDispatchCompute(ceil(inputTexture->width() / 8.0f), ceil(inputTexture->height() / 4.0f), 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glFinish();

        return true;
    }

}
