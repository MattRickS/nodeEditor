#pragma once
#include <string>
#include <vector>

#include "../constants.h"
#include "../gl/RenderSetOperator.h"
#include "../nodegraph/Settings.h"
#include "../nodegraph/OperatorRegistry.hpp"

namespace Op
{
    class ExtractLayer : public RenderSetOperator
    {
    public:
        static ExtractLayer *create()
        {
            return new ExtractLayer();
        }

        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerString("fromLayer", DEFAULT_LAYER);
        }

        bool process(const std::vector<RenderSetOperator const *> &inputs,
                     Settings const *settings,
                     [[maybe_unused]] Settings const *sceneSettings) override
        {
            std::string fromLayer = settings->getString("fromLayer");
            Texture const *tex = inputs[0]->layer(fromLayer);
            if (!tex)
            {
                setError("Input does not contain the requested fromLayer");
                return false;
            }

            m_renderSet.clear();
            m_renderSet[DEFAULT_LAYER] = tex;
            return true;
        }
    };

    REGISTER_OPERATOR(ExtractLayer, ExtractLayer::create);
}
