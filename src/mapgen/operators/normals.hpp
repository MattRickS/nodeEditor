#pragma once
#include <string>
#include <vector>

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
        void defaultSettings(Settings *settings) const override
        {
            settings->registerFloat("scale", true);
            settings->registerUInt("channel", 0);
        }
    };

    REGISTER_OPERATOR(Normals, Normals::create);
}
