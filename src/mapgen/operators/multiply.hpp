#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class Multiply : public BaseComputeShaderOp
    {
    public:
        static Multiply *create()
        {
            return new Multiply();
        }

        Multiply() : BaseComputeShaderOp("src/mapgen/operators/multiply.glsl") {}
        std::string name() const override { return "Multiply"; }
        std::vector<Input> inputs() const override
        {
            return {{}, {"Multiplier", false}};
        }
        void defaultSettings(Settings *settings) const override
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
