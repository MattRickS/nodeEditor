#include <iostream>
#include <vector>

#include <GL/glew.h>

#include "operator.h"
#include "renders.h"
#include "util.h"

namespace Op
{
    std::vector<Input> Operator::inputs() const { return {}; }
    std::vector<Output> Operator::outputs() const { return {{}}; }
    void Operator::defaultSettings([[maybe_unused]] Settings *settings) const {}

    void Operator::preprocess([[maybe_unused]] const std::vector<Texture *> &inputs, [[maybe_unused]] const std::vector<Texture *> &outputs, [[maybe_unused]] const Settings *settings) {}
    const std::string &Operator::error() { return m_error; }

    void Operator::setError(std::string errorMsg) { m_error = errorMsg; }
    bool Operator::hasError() const { return !m_error.empty(); }
    void Operator::reset() { m_error.clear(); }

    BaseComputeShaderOp::BaseComputeShaderOp(const char *computeShader) : shader(computeShader) {}
    bool BaseComputeShaderOp::process(const std::vector<Texture *> &inputs,
                                      const std::vector<Texture *> &outputs,
                                      const Settings *settings)
    {
        // Setup shader
        shader.use();
        for (auto it = settings->cbegin(); it != settings->cend(); ++it)
        {
            switch (it->type())
            {
            case SettingType_Bool:
                shader.setBool(it->name(), it->value<bool>());
                DEBUG_LOG("Setting %s to %s", it->name().c_str(), it->value<bool>() ? "true" : "false");
                break;
            case SettingType_Float:
                shader.setFloat(it->name(), it->value<float>());
                DEBUG_LOG("Setting %s to %.3f", it->name().c_str(), it->value<float>());
                break;
            case SettingType_Float2:
                shader.setVec2(it->name(), it->value<glm::vec2>());
                DEBUG_LOG("Setting %s to (%.3f, %.3f)", it->name().c_str(), it->value<glm::vec2>().x, it->value<glm::vec2>().y);
                break;
            case SettingType_Float3:
                shader.setVec3(it->name(), it->value<glm::vec3>());
                DEBUG_LOG("Setting %s to (%.3f, %.3f, %.3f)", it->name().c_str(), it->value<glm::vec3>().x, it->value<glm::vec3>().y, it->value<glm::vec3>().z);
                break;
            case SettingType_Float4:
                shader.setVec4(it->name(), it->value<glm::vec4>());
                DEBUG_LOG("Setting %s to (%.3f, %.3f, %.3f, %.3f)", it->name().c_str(), it->value<glm::vec4>().x, it->value<glm::vec4>().y, it->value<glm::vec4>().z, it->value<glm::vec4>().w);
                break;
            case SettingType_Int:
                shader.setInt(it->name(), it->value<int>());
                DEBUG_LOG("Setting %s to %d", it->name().c_str(), it->value<int>());
                break;
            case SettingType_Int2:
                shader.setIVec2(it->name(), it->value<glm::ivec2>());
                DEBUG_LOG("Setting %s to (%d, %d)", it->name().c_str(), it->value<glm::ivec2>().x, it->value<glm::ivec2>().y);
                break;
            case SettingType_UInt:
                shader.setUInt(it->name(), it->value<unsigned int>());
                DEBUG_LOG("Setting %s to %u", it->name().c_str(), it->value<unsigned int>());
                break;
            }
        }

        size_t i = 0;
        for (; i < inputs.size(); ++i)
        {
            auto inTex = inputs[i];
            // Optional inputs might be a nullptr
            if (inTex)
            {
                DEBUG_LOG("Binding input %lu to ID: %u", i, inTex->ID);
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, inTex->ID);
                glBindImageTexture(i, inTex->ID, 0, GL_FALSE, 0, GL_READ_ONLY, inTex->internalFormat());
                // TODO: Needs to check if it's optional, otherwise this setting won't exist
                // Internal naming convention for disabling optional inputs
                shader.setBool("_ignoreImage" + std::to_string(i), false);
            }
            else
            {
                DEBUG_LOG("Ignoring input %lu", i);
                // Internal naming convention for disabling optional inputs
                shader.setBool("_ignoreImage" + std::to_string(i), true);
            }
        }

        auto outTex = outputs[0];
        DEBUG_LOG("Binding output %lu to ID: %u", i, outTex->ID);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, outTex->ID);
        glBindImageTexture(i, outTex->ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, outTex->internalFormat());

        // Render
        glDispatchCompute(ceil(outTex->width / 8), ceil(outTex->height / 4), 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        return true;
    }
}
