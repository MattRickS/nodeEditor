#pragma once
#include <string>
#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Shuffle : public ComputeShaderOperator
    {
    public:
        static Shuffle *create()
        {
            return new Shuffle();
        }

        Shuffle() : ComputeShaderOperator("src/mapgen/operators/Shuffle.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerInt("red", ChannelMask_Red, ChannelMask_None, ChannelMask_Alpha, SettingHint_ChannelMask);
            settings->registerInt("green", ChannelMask_Green, ChannelMask_None, ChannelMask_Alpha, SettingHint_ChannelMask);
            settings->registerInt("blue", ChannelMask_Blue, ChannelMask_None, ChannelMask_Alpha, SettingHint_ChannelMask);
            settings->registerInt("alpha", ChannelMask_Alpha, ChannelMask_None, ChannelMask_Alpha, SettingHint_ChannelMask);
            settings->registerInt("white", ChannelMask_None, ChannelMask_None, ChannelMask_Alpha, SettingHint_ChannelMask);
            settings->registerInt("black", ChannelMask_None, ChannelMask_None, ChannelMask_Alpha, SettingHint_ChannelMask);
        }
    };

    REGISTER_OPERATOR(Shuffle, Shuffle::create);
}
