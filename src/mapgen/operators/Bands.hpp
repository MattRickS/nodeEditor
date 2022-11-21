#pragma once
#include <string>
#include <vector>

#include "../gl/ContentCreatorComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Bands : public ContentCreatorComputeShaderOperator
    {
    public:
        static Bands *create()
        {
            return new Bands();
        }

        Bands() : ContentCreatorComputeShaderOperator("src/mapgen/operators/Bands.glsl") {}
        void registerSettings(Settings *const settings) const override
        {
            ContentCreatorComputeShaderOperator::registerSettings(settings);
            settings->registerInt("divisionSize", 0, 0, 100);
            settings->registerFloat4("divisionColour", {0.0f, 0.0f, 0.0f, 1.0f}, 0.0f, 1.0f, SettingHint_Color);
            settings->registerFloat4Array("bands", {{1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}, 0.0f, 1.0f, SettingHint_Color);
        }
    };

    REGISTER_OPERATOR(Bands, Bands::create);
}
