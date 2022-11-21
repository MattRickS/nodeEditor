#pragma once
#include <string>
#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    /*
    For pixels with matching values, sets their rg components to the pixel xy pos.
    This can be used as a source for JumpFlood.
    */
    class Pixel : public ComputeShaderOperator
    {
    public:
        static Pixel *create()
        {
            return new Pixel();
        }

        Pixel() : ComputeShaderOperator("src/nodeeditor/operators/Pixel.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerInt("channel", 0, 0, 3, SettingHint_Channel);
            settings->registerFloat("minValue", 0.0f, 0.0f, 1.0f);
            settings->registerFloat("maxValue", 1.0f, 0.0f, 1.0f);
        }
    };

    REGISTER_OPERATOR(Pixel, Pixel::create);
}
