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
        size_t binding = 0;
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
            case SettingType_Float2Array:
                bindSSBO(binding++, *it);
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
                    bindImage(i, texture, GL_READ_ONLY);
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
            bindImage(i + j, outTex, GL_WRITE_ONLY);
            imageSize = glm::max(imageSize, definedOutputs[j].imageSize);
        }

        // Render
        render(imageSize);

        return true;
    }

    void ComputeShaderOperator::render(glm::ivec2 imageSize)
    {
        glDispatchCompute(ceil(imageSize.x / 8.0f), ceil(imageSize.y / 4.0f), 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glFinish();
    }
    void ComputeShaderOperator::bindSSBO(size_t index, const Setting &setting)
    {
        // SSBOs are only created once
        if (index >= m_ssbos.size())
        {
            m_ssbos.emplace_back();
        }
        // index in the vector and it's binding point are the same
        if (m_ssbos[index].load(setting, index))
        {
            LOG_DEBUG("Setting %s bound to index %lu", setting.name().c_str(), index);
        }
        else
        {
            LOG_WARNING("Failed to bind setting %s to SSBO index %lu", setting.name().c_str(), index);
        }
    }
}
