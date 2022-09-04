#pragma once
#include <string>
#include <vector>

#include "../constants.h"
#include "../gl/RenderSetOperator.h"
#include "../nodegraph/Settings.h"
#include "../nodegraph/OperatorRegistry.hpp"

namespace Op
{
    class CopyLayer : public RenderSetOperator
    {
    public:
        static CopyLayer *create()
        {
            return new CopyLayer();
        }

        std::vector<Input> inputs() const override
        {
            return {{"Left"}, {"Right"}};
        }
        void registerSettings(Settings *const settings) const override
        {
            // TODO: How would these register choices based on inputs? SettingHint?
            settings->registerString("fromLayer", DEFAULT_LAYER);
            settings->registerString("toLayer", DEFAULT_LAYER);
        }

        bool process(const std::vector<RenderSetOperator const *> &inputs,
                     Settings const *settings,
                     [[maybe_unused]] Settings const *sceneSettings) override
        {
            std::string fromLayer = settings->getString("fromLayer");
            std::string toLayer = settings->getString("toLayer");
            if (fromLayer.empty() || toLayer.empty())
            {
                setError("Must specify both from and to layers");
                return false;
            }

            Texture const *tex = inputs[1]->layer(fromLayer);
            if (!tex)
            {
                setError("Right input does not contain the requested fromLayer");
                return false;
            }

            m_renderSet[toLayer] = tex;
            return true;
        }
    };

    REGISTER_OPERATOR(CopyLayer, CopyLayer::create);
}
