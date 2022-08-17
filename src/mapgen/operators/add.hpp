#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../renders.h"
#include "../settings.h"
#include "../shader.h"

namespace Op
{
    class Add : public BaseComputeShaderOp
    {
    public:
        static Add *create()
        {
            return new Add();
        }

        Add() : BaseComputeShaderOp("src/mapgen/operators/add.glsl") {}
        std::string name() const override { return "Add"; }
        std::vector<Input> inputs() const override
        {
            return {{}, {"Add", false}};
        }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerBool("red", true);
            settings->registerBool("green", true);
            settings->registerBool("blue", true);
            settings->registerBool("alpha", false);
            settings->registerFloat("value", 0.0f);
        }
    };

    REGISTER_OPERATOR(Add, Add::create);
}
