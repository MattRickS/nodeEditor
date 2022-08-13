#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../renders.h"
#include "../settings.h"
#include "../shader.h"

class AddOp : public BaseComputeShaderOp
{
public:
    static AddOp *create()
    {
        return new AddOp();
    }

    AddOp() : BaseComputeShaderOp("src/mapgen/shaders/compute/add.glsl") {}
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

REGISTER_OPERATOR(AddOp, AddOp::create);
