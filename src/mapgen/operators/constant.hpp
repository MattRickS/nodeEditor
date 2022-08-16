#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../renders.h"
#include "../settings.h"
#include "../shader.h"

namespace Op
{
    class Constant : public BaseComputeShaderOp
    {
    public:
        static Constant *create()
        {
            return new Constant();
        }

        Constant() : BaseComputeShaderOp("src/mapgen/shaders/compute/constant.glsl") {}
        std::string name() const override { return "Constant"; }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerFloat4("color", glm::vec4(0, 0, 0, 1));
        }
    };

    REGISTER_OPERATOR(Constant, Constant::create);
}