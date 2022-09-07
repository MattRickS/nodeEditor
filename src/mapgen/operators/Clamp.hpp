#pragma once
#include <string>
#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Clamp : public ComputeShaderOperator
    {
    public:
        static Clamp *create()
        {
            return new Clamp();
        }

        Clamp() : ComputeShaderOperator("src/mapgen/operators/Clamp.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerFloat("minValue", 0.0f, 0.0f, 1.0f);
            settings->registerFloat("maxValue", 1.0f, 0.0f, 1.0f);
        }
    };

    REGISTER_OPERATOR(Clamp, Clamp::create);
}
