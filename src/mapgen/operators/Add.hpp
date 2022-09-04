#pragma once
#include <string>
#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Add : public ComputeShaderOperator
    {
    public:
        static Add *create()
        {
            return new Add();
        }

        Add() : ComputeShaderOperator("src/mapgen/operators/Add.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerInt("channelMask", ChannelMask_RGB, ChannelMask_None, ChannelMask_Alpha, SettingHint_ChannelMask);
            settings->registerFloat("add", 0.0f);
        }
    };

    REGISTER_OPERATOR(Add, Add::create);
}
