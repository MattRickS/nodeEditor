#pragma once
#include <string>
#include <vector>

#include "../gl/ContentCreatorComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Gradient : public ContentCreatorComputeShaderOperator
    {
    public:
        static Gradient *create()
        {
            return new Gradient();
        }

        Gradient() : ContentCreatorComputeShaderOperator("src/mapgen/operators/Gradient.glsl") {}
        void registerSettings(Settings *const settings) const override
        {
            ContentCreatorComputeShaderOperator::registerSettings(settings);
            settings->registerFloat2("start", glm::vec2(0));
            settings->registerFloat2("end", glm::vec2(100));
            settings->registerFloat4("startColour", glm::vec4(1), 0.0f, 1.0f, SettingHint_Color);
            settings->registerFloat4("endColour", glm::vec4(0), 0.0f, 1.0f, SettingHint_Color);
            settings->registerFloat("falloff", 1.0f);
        }
    };

    REGISTER_OPERATOR(Gradient, Gradient::create);
}
