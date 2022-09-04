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
            return {{}};
        }
        std::vector<OutputLayer> outputLayers(const std::vector<RenderSetOperator const *> &inputs, [[maybe_unused]] Settings const *settings, Settings const *sceneSettings) override
        {
            return {{"Temperature", outputLayerSize(0, inputs, sceneSettings)}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerUInt("loBand", 0);
            settings->registerUInt("hiBand", 0);
            settings->registerFloat("falloff", 0.5f);
        }
    };

    REGISTER_OPERATOR(Temperature, Temperature::create);
}
