#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class Constant : public ContentCreatorComputeShaderOp
    {
    public:
        static Constant *create()
        {
            return new Constant();
        }

        Constant() : ContentCreatorComputeShaderOp("src/mapgen/operators/constant.glsl") {}
        std::string name() const override { return "Constant"; }
        void defaultSettings(Settings *const settings) const override
        {
            ContentCreatorComputeShaderOp::defaultSettings(settings);
            settings->registerFloat4("color", glm::vec4(0, 0, 0, 1), 0.0f, 1.0f, SettingHint_Color);
        }
    };

    REGISTER_OPERATOR(Constant, Constant::create);
}
