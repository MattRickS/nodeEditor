#pragma once
#include <string>
#include <vector>

#include "../gl/ConvolveOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class ConvolveTexture : public ConvolveOperator
    {
    public:
        static ConvolveTexture *create()
        {
            return new ConvolveTexture();
        }

        std::vector<Input> inputs() const override
        {
            return {{"Image"}, {"Kernel"}};
        }
        void registerSettings(Settings *const settings) const override
        {
            ConvolveOperator::registerSettings(settings);
            settings->registerInt("channel", 0, 0, 2, SettingHint_Channel);
            settings->registerBool("normalise", true);
        }
        bool populateKernel(ConvolveKernel *kernel,
                            const std::vector<RenderSetOperator const *> &inputs,
                            [[maybe_unused]] Settings const *settings,
                            [[maybe_unused]] Settings const *sceneSettings) override
        {
            Texture const *texture = inputs[1]->layer(DEFAULT_LAYER);
            if (!texture)
            {
                setError("Missing default layer for Kernel");
                return false;
            }

            kernel->readFromTexture(*texture, Channel(settings->getInt("channel")));

            if (settings->getBool("normalise"))
            {
                kernel->normalise();
            }

            return true;
        }
    };

    REGISTER_OPERATOR(ConvolveTexture, ConvolveTexture::create);
}
