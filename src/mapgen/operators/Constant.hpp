#pragma once
#include <string>
#include <vector>

#include "../gl/ContentCreatorComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Constant : public ContentCreatorComputeShaderOperator
    {
    public:
        static Constant *create()
        {
            return new Constant();
        }

        Constant() : ContentCreatorComputeShaderOperator("src/mapgen/operators/Constant.glsl") {}
        void registerSettings(Settings *const settings) const override
        {
            ContentCreatorComputeShaderOperator::registerSettings(settings);
            settings->registerFloat4("color", glm::vec4(0, 0, 0, 1), 0.0f, 1.0f, SettingHint_Color);
        }
    };

    REGISTER_OPERATOR(Constant, Constant::create);
}
