#pragma once
#include <string>
#include <vector>

#include "../constants.h"
#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class Normals : public BaseComputeShaderOp
    {
    public:
        static Normals *create()
        {
            return new Normals();
        }

        Normals() : BaseComputeShaderOp("src/mapgen/operators/normals.glsl") {}
        std::string name() const override { return "Normals"; }
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void defaultSettings(Settings *const settings) const override
        {
            settings->registerFloat("scale", true, 0.01f, 100.0f);
            settings->registerInt("channel", ::Channel_Red, 0, 3, SettingHint_Channel);
        }
    };

    REGISTER_OPERATOR(Normals, Normals::create);
}
