#pragma once
#include <string>
#include <vector>

#include "../constants.h"
#include "../nodegraph/Settings.h"
#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"

namespace Op
{
    class Normals : public ComputeShaderOperator
    {
    public:
        static Normals *create()
        {
            return new Normals();
        }

        Normals() : ComputeShaderOperator("src/nodeeditor/operators/Normals.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerFloat("scale", true, 0.01f, 100.0f);
            settings->registerInt("channel", ::Channel_Red, 0, 3, SettingHint_Channel);
        }
    };

    REGISTER_OPERATOR(Normals, Normals::create);
}
