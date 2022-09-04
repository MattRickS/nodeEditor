#include "../log.h"
#include "../nodegraph/Operator.h"
#include "RenderSetOperator.h"

namespace Op
{
    RenderSetOperator::~RenderSetOperator()
    {
        // Outputs are owned by the operator and must be cleaned up when destroyed
        for (auto &[key, value] : m_outputs)
        {
            delete value;
        }
    }

    RenderSet const *RenderSetOperator::renderSet() const { return &m_renderSet; }

    Texture const *RenderSetOperator::layer(const std::string &layer) const
    {
        auto it = m_renderSet.find(layer);
        if (it == m_renderSet.end())
        {
            return nullptr;
        }
        return it->second;
    }

    void RenderSetOperator::reset()
    {
        Operator::reset();
        m_renderSet.clear();
        m_renderSetConfigured = false;
    }

    glm::ivec2 RenderSetOperator::outputLayerSize([[maybe_unused]] int outputIndex, const std::vector<RenderSetOperator const *> &inputs, Settings const *sceneSettings)
    {
        if (!inputs.empty())
        {
            Texture const *texture = inputs[0]->layer(DEFAULT_LAYER);
            return {texture->width(), texture->height()};
        }
        return sceneSettings->getInt2(SCENE_SETTING_IMAGE_SIZE);
    }

    Texture *RenderSetOperator::ensureOutputLayer(const std::string &layer, const glm::ivec2 &imageSize)
    {
        Texture *tex;
        auto it = m_outputs.find(layer);
        if (it == m_outputs.end())
        {
            m_outputs.emplace(layer, new Texture(imageSize.x, imageSize.y));
            tex = m_outputs[layer];
            LOG_DEBUG("Created output ID %u for layer %s", m_outputs[layer]->id(), layer.c_str());
        }
        else if (it->second->width() != (unsigned int)imageSize.x || it->second->height() != (unsigned int)imageSize.y)
        {
            it->second->resize(imageSize.x, imageSize.y);
            tex = it->second;
            LOG_DEBUG("Resized output ID %u for layer %s to (%u, %u)", tex->id(), layer.c_str(), imageSize.x, imageSize.y);
        }
        else
        {
            tex = it->second;
        }
        // Ensure the output renderset's layer points to this Operator's texture.
        m_renderSet[layer] = tex;
        return tex;
    }

    bool RenderSetOperator::process(const std::vector<Operator const *> &inputs, Settings const *settings, Settings const *sceneSettings)
    {
        if (!m_renderSetConfigured)
        {
            m_renderSet.clear();
            // Copy the first input's (if any) renderset into this operator's renderset
            if (inputs.size() > 0)
            {
                RenderSetOperator const *op = dynamic_cast<RenderSetOperator const *>(inputs[0]);
                if (!op)
                {
                    setError("Input is not an instance of RenderSetOperator");
                    return false;
                }
                for (const auto &[key, value] : *op->renderSet())
                {
                    m_renderSet[key] = value;
                }
            }
        }

        const auto &definedInputs = this->inputs();
        std::vector<RenderSetOperator const *> renderSets;
        for (size_t i = 0; i < inputs.size(); ++i)
        {
            const RenderSetOperator *input = dynamic_cast<const RenderSetOperator *>(inputs[i]);
            // Optional inputs might be a nullptr
            if (input)
            {
                renderSets.emplace_back(input);
            }
            else if (definedInputs[i].required)
            {
                LOG_ERROR("Required input missing: %s", definedInputs[i].name.c_str());
                return false;
            }
            else if (inputs[i])
            {
                LOG_ERROR("Input is not a RenderSetOperator: %s", definedInputs[i].name.c_str());
                return false;
            }
            else
            {
                LOG_DEBUG("No input provided for: %s", definedInputs[i].name.c_str());
                renderSets.emplace_back(nullptr);
            }
        }

        return process(renderSets, settings, sceneSettings);
    }
}
