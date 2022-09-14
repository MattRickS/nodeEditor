#pragma once
#include <string>
#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Temperature : public ComputeShaderOperator
    {
    public:
        static Temperature *create()
        {
            return new Temperature();
        }

        Temperature() : ComputeShaderOperator("src/mapgen/operators/Temperature.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{}, {"WaterDistance", false}};
        }
        std::vector<OutputLayer> outputLayers(const std::vector<RenderSetOperator const *> &inputs, [[maybe_unused]] Settings const *settings, Settings const *sceneSettings) override
        {
            return {{"Temperature", outputLayerSize(0, inputs, sceneSettings)}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerFloat("loFalloff", 0.5f, 0.0f, 1.0f);
            settings->registerFloat("hiFalloff", 0.5f, 0.0f, 1.0f);
            settings->registerFloat("heightMult", 1.0f, 0.0f, 1.0f);
            settings->registerFloat("waterMult", 1.0f, 0.0f, 1.0f);
        }
    };

    REGISTER_OPERATOR(Temperature, Temperature::create);
}
