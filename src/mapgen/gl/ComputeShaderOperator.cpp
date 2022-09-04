#include "../log.h"
#include "ComputeShaderOperator.h"

namespace Op
{
    ComputeShaderOperator::ComputeShaderOperator(const char *computeShader) : RenderSetOperator(), m_shader(computeShader) {}
    std::vector<OutputLayer> ComputeShaderOperator::outputLayers(const std::vector<RenderSetOperator const *> &inputs, [[maybe_unused]] Settings const *settings, Settings const *sceneSettings)
    {
        return {{DEFAULT_LAYER, outputLayerSize(0, inputs, sceneSettings)}};
    }
    bool ComputeShaderOperator::process(const std::vector<RenderSetOperator const *> &inputs, Settings const *settings, Settings const *sceneSettings)
    {
        m_shader.use();

        // Add all settings to the shader. Assumes identical names.
        for (auto it = settings->cbegin(); it != settings->cend(); ++it)
        {
            switch (it->type())
            {
            case SettingType_Bool:
                m_shader.setBool(it->name(), it->value<bool>());
                LOG_DEBUG("Setting %s to %s", it->name().c_str(), it->value<bool>() ? "true" : "false");
                break;
            case SettingType_Float:
                m_shader.setFloat(it->name(), it->value<float>());
                LOG_DEBUG("Setting %s to %.3f", it->name().c_str(), it->value<float>());
                break;
            case SettingType_Float2:
                m_shader.setVec2(it->name(), it->value<glm::vec2>());
                LOG_DEBUG("Setting %s to (%.3f, %.3f)", it->name().c_str(), it->value<glm::vec2>().x, it->value<glm::vec2>().y);
                break;
            case SettingType_Float3:
                m_shader.setVec3(it->name(), it->value<glm::vec3>());
                LOG_DEBUG("Setting %s to (%.3f, %.3f, %.3f)", it->name().c_str(), it->value<glm::vec3>().x, it->value<glm::vec3>().y, it->value<glm::vec3>().z);
                break;
            case SettingType_Float4:
                m_shader.setVec4(it->name(), it->value<glm::vec4>());
                LOG_DEBUG("Setting %s to (%.3f, %.3f, %.3f, %.3f)", it->name().c_str(), it->value<glm::vec4>().x, it->value<glm::vec4>().y, it->value<glm::vec4>().z, it->value<glm::vec4>().w);
                break;
            case SettingType_Int:
                m_shader.setInt(it->name(), it->value<int>());
                LOG_DEBUG("Setting %s to %d", it->name().c_str(), it->value<int>());
                break;
            case SettingType_Int2:
                m_shader.setIVec2(it->name(), it->value<glm::ivec2>());
                LOG_DEBUG("Setting %s to (%d, %d)", it->name().c_str(), it->value<glm::ivec2>().x, it->value<glm::ivec2>().y);
                break;
            case SettingType_UInt:
                m_shader.setUInt(it->name(), it->value<unsigned int>());
                LOG_DEBUG("Setting %s to %u", it->name().c_str(), it->value<unsigned int>());
                break;
            case SettingType_String:
                // glsl has no string type
                LOG_WARNING("Ignoring string setting %s", it->name().c_str());
                break;
            }
        }

        // Ensure each input is bound sequentially to the shader
        const auto &definedInputs = this->inputs();
        size_t i = 0;
        for (; i < inputs.size(); ++i)
        {
            // Optional inputs might be a nullptr
            if (inputs[i])
            {
                const Texture *texture = inputs[i]->layer(DEFAULT_LAYER);
                if (texture)
                {
                    LOG_DEBUG("Binding input %lu to ID: %u", i, texture->id());
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, texture->id());
                    glBindImageTexture(i, texture->id(), 0, GL_FALSE, 0, GL_READ_ONLY, texture->internalFormat());
                    // Internal naming convention for disabling optional inputs in the shader
                    if (!definedInputs[i].required)
                    {
                        m_shader.setBool("_ignoreImage" + std::to_string(i), false);
                    }
                }
                else
                {
                    LOG_DEBUG("Input is connected but does not provide the default layer: %s", definedInputs[i].name.c_str());
                }
            }
            else
            {
                // Internal naming convention for disabling optional inputs
                m_shader.setBool("_ignoreImage" + std::to_string(i), true);
            }
        }

        // Ensure there are textures generated and bound for each defined output
        glm::ivec2 imageSize(0);
        const auto &definedOutputs = outputLayers(inputs, settings, sceneSettings);
        for (size_t j = 0; j < definedOutputs.size(); ++j)
        {
            Texture *outTex = ensureOutputLayer(definedOutputs[j].layer, definedOutputs[j].imageSize);
            LOG_DEBUG("Binding output %lu to ID: %u", j, outTex->id());
            glActiveTexture(GL_TEXTURE0 + i + j);
            glBindTexture(GL_TEXTURE_2D, outTex->id());
            glBindImageTexture(i + j, outTex->id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, outTex->internalFormat());
            imageSize = glm::max(imageSize, definedOutputs[j].imageSize);
        }

        // Render
        glDispatchCompute(ceil(imageSize.x / 8), ceil(imageSize.y / 4), 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glFinish();

        return true;
    }
}
