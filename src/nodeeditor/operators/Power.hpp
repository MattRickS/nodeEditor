#pragma once
#include <string>
#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Power : public ComputeShaderOperator
    {
    public:
        static Power *create()
        {
            return new Power();
        }

        Power() : ComputeShaderOperator("src/nodeeditor/operators/Power.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerInt("channelMask", ChannelMask_RGB, ChannelMask_Red, ChannelMask_Alpha, SettingHint_ChannelMask);
            settings->registerFloat("exponent", 2.0f, 0.0f, 10.0f);
        }
    };

    REGISTER_OPERATOR(Power, Power::create);
}
