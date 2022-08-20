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
        std::vector<Input> inputs() const override
        {
            return {{"", false}};
        }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerFloat2("start", glm::vec2(0));
            settings->registerFloat2("end", glm::vec2(100));
            settings->registerFloat4("startColour", glm::vec4(1));
            settings->registerFloat4("endColour", glm::vec4(0));
            settings->registerFloat("falloff", 1.0f);
        }
    };

    REGISTER_OPERATOR(Gradient, Gradient::create);
}
