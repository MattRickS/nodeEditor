#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../renders.h"
#include "../settings.h"
#include "../shader.h"

class MultiplyOp : public BaseComputeShaderOp
{
public:
    static MultiplyOp *create()
    {
        return new MultiplyOp();
    }

    MultiplyOp() : BaseComputeShaderOp("src/mapgen/shaders/compute/multiply.glsl") {}
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

REGISTER_OPERATOR(MultiplyOp, MultiplyOp::create);
