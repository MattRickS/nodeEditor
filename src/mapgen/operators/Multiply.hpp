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
            settings->registerInt("channelMask", ChannelMask_RGB, ChannelMask_Red, ChannelMask_Alpha, SettingHint_ChannelMask);
            settings->registerFloat("multiplier", 1.0f);
        }
    };

    REGISTER_OPERATOR(Multiply, Multiply::create);
}
