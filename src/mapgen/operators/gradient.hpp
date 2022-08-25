#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class Gradient : public BaseComputeShaderOp
    {
    public:
        static Gradient *create()
        {
            return new Gradient();
        }

        Gradient() : BaseComputeShaderOp("src/mapgen/operators/gradient.glsl") {}
        std::string name() const override { return "Gradient"; }
        void defaultSettings(Settings *const settings) const override
        {
            settings->registerFloat2("start", glm::vec2(0));
            settings->registerFloat2("end", glm::vec2(100));
            settings->registerFloat4("startColour", glm::vec4(1), 0.0f, 1.0f, SettingHint_Color);
            settings->registerFloat4("endColour", glm::vec4(0), 0.0f, 1.0f, SettingHint_Color);
            settings->registerFloat("falloff", 1.0f);
        }
    };

    REGISTER_OPERATOR(Gradient, Gradient::create);
}
