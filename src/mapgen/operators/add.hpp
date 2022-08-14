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

        Add() : BaseComputeShaderOp("src/mapgen/shaders/compute/add.glsl") {}
        std::string name() const override { return "Add"; }
        std::vector<Input> inputs() const override
        {
            return {{}, {}};
        }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerBool("red", true);
            settings->registerBool("green", true);
            settings->registerBool("blue", true);
            settings->registerBool("alpha", false);
        }
    };

    REGISTER_OPERATOR(Add, Add::create);
}
