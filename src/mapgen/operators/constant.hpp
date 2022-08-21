#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class Constant : public BaseComputeShaderOp
    {
    public:
        static Constant *create()
        {
            return new Constant();
        }

        Constant() : BaseComputeShaderOp("src/mapgen/operators/constant.glsl") {}
        std::string name() const override { return "Constant"; }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerFloat4("color", glm::vec4(0, 0, 0, 1), SettingHint_Color);
        }
    };

    REGISTER_OPERATOR(Constant, Constant::create);
}
