#include <vector>

#include <GL/glew.h>

#include "constants.h"
#include "operator.h"
#include "renders.h"
#include "util.h"

namespace Op
{
    std::vector<Input> Operator::inputs() const { return {}; }
    std::vector<Output> Operator::outputs() const { return {{}}; }
    glm::ivec2 Operator::outputImageSize(const std::vector<Texture *> &inputs, const Settings *const sceneSettings)
    {
        if (!inputs.empty())
        {
            return {inputs[0]->width(), inputs[0]->height()};
        }
        LOG_DEBUG("Using scene image size for %s", name().c_str());
        return sceneSettings->getInt2(SCENE_SETTING_IMAGE_SIZE);
    }
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
                LOG_DEBUG("Setting %s to %s", it->name().c_str(), it->value<bool>() ? "true" : "false");
                break;
            case SettingType_Float:
                shader.setFloat(it->name(), it->value<float>());
                LOG_DEBUG("Setting %s to %.3f", it->name().c_str(), it->value<float>());
                break;
            case SettingType_Float2:
                shader.setVec2(it->name(), it->value<glm::vec2>());
                LOG_DEBUG("Setting %s to (%.3f, %.3f)", it->name().c_str(), it->value<glm::vec2>().x, it->value<glm::vec2>().y);
                break;
            case SettingType_Float3:
                shader.setVec3(it->name(), it->value<glm::vec3>());
                LOG_DEBUG("Setting %s to (%.3f, %.3f, %.3f)", it->name().c_str(), it->value<glm::vec3>().x, it->value<glm::vec3>().y, it->value<glm::vec3>().z);
                break;
            case SettingType_Float4:
                shader.setVec4(it->name(), it->value<glm::vec4>());
                LOG_DEBUG("Setting %s to (%.3f, %.3f, %.3f, %.3f)", it->name().c_str(), it->value<glm::vec4>().x, it->value<glm::vec4>().y, it->value<glm::vec4>().z, it->value<glm::vec4>().w);
                break;
            case SettingType_Int:
                shader.setInt(it->name(), it->value<int>());
                LOG_DEBUG("Setting %s to %d", it->name().c_str(), it->value<int>());
                break;
            case SettingType_Int2:
                shader.setIVec2(it->name(), it->value<glm::ivec2>());
                LOG_DEBUG("Setting %s to (%d, %d)", it->name().c_str(), it->value<glm::ivec2>().x, it->value<glm::ivec2>().y);
                break;
            case SettingType_UInt:
                shader.setUInt(it->name(), it->value<unsigned int>());
                LOG_DEBUG("Setting %s to %u", it->name().c_str(), it->value<unsigned int>());
                break;
            case SettingType_String:
                // glsl has no string type
                LOG_WARNING("Ignoring string setting %s", it->name().c_str());
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
                LOG_DEBUG("Binding input %lu to ID: %u", i, inTex->id());
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, inTex->id());
                glBindImageTexture(i, inTex->id(), 0, GL_FALSE, 0, GL_READ_ONLY, inTex->internalFormat());
                // TODO: Needs to check if it's optional, otherwise this setting won't exist
                // Internal naming convention for disabling optional inputs
                shader.setBool("_ignoreImage" + std::to_string(i), false);
            }
            else
            {
                LOG_DEBUG("Ignoring input %lu", i);
                // Internal naming convention for disabling optional inputs
                shader.setBool("_ignoreImage" + std::to_string(i), true);
            }
        }

        auto outTex = outputs[0];
        LOG_DEBUG("Binding output %lu to ID: %u", i, outTex->id());
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, outTex->id());
        glBindImageTexture(i, outTex->id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, outTex->internalFormat());

        // Render
        glDispatchCompute(ceil(outTex->width() / 8), ceil(outTex->height() / 4), 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        return true;
    }
}
