#pragma once
#include <string>
#include <vector>

#include "../gl/ContentCreatorComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class CheckerBoard : public ContentCreatorComputeShaderOperator
    {
    public:
        static CheckerBoard *create()
        {
            return new CheckerBoard();
        }

        CheckerBoard() : ContentCreatorComputeShaderOperator("src/nodeeditor/operators/CheckerBoard.glsl") {}
        void registerSettings(Settings *const settings) const override
        {
            ContentCreatorComputeShaderOperator::registerSettings(settings);
            settings->registerUInt("size", 128, 0, 2048);
            settings->registerFloat4("color1", {0.0f, 0.0f, 0.0f, 1.0f}, 0.0f, 1.0f, SettingHint_Color);
            settings->registerFloat4("color2", {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, 1.0f, SettingHint_Color);
        }
    };

    REGISTER_OPERATOR(CheckerBoard, CheckerBoard::create);
}
