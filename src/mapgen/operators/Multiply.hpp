#pragma once
#include <string>
#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Multiply : public ComputeShaderOperator
    {
    public:
        static Multiply *create()
        {
            return new Multiply();
        }

        Multiply() : ComputeShaderOperator("src/mapgen/operators/Multiply.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerBool("red", true);
            settings->registerBool("green", true);
            settings->registerBool("blue", true);
            settings->registerBool("alpha", false);
            settings->registerFloat("multiplier", 1.0f);
        }
    };

    REGISTER_OPERATOR(Multiply, Multiply::create);
}
