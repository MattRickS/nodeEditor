#pragma once
#include <vector>

#include "../gl/ConvolveOperator.h"
#include "../gl/ConvolveKernel.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"
#include "../constants.h"
#include "../log.h"

namespace Op
{
    class Gaussian : public ConvolveOperator
    {
    public:
        static Gaussian *create()
        {
            return new Gaussian();
        }

        void registerSettings(Settings *const settings) const override
        {
            ConvolveOperator::registerSettings(settings);
            settings->registerInt("radius", 3, 1, 30);
            settings->registerFloat("sigma", 3.0f, 1.0f, 10.0f);
        }
        bool populateKernel(ConvolveKernel *kernel,
                            [[maybe_unused]] const std::vector<RenderSetOperator const *> &inputs,
                            Settings const *settings,
                            [[maybe_unused]] Settings const *sceneSettings) override
        {
            int radius = settings->getInt("radius");
            float sigma = settings->getFloat("sigma");

            unsigned int maskSize = radius * 2 + 1;
            kernel->resize(maskSize, maskSize);

            float sum = 0.0f;
            for (int y = -radius; y < radius + 1; y++)
            {
                for (int x = -radius; x < radius + 1; x++)
                {
                    float pow = (float)(y * y + x * x) / (2 * sigma * sigma);
                    float weight = exp(-pow) * (1.0f / (2 * PI * sigma * sigma));
                    int index = y + radius + (x + radius) * maskSize;
                    (*kernel)[index] = weight;
                    sum += weight;
                }
            }

            for (unsigned int i = 0; i < maskSize * maskSize; ++i)
            {
                (*kernel)[i] /= sum;
            }

            return true;
        }
    };

    REGISTER_OPERATOR(Gaussian, Gaussian::create);
}
