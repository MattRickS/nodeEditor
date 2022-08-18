#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class Temperature : public BaseComputeShaderOp
    {
    public:
        static Temperature *create()
        {
            return new Temperature();
        }

        Temperature() : BaseComputeShaderOp("src/mapgen/operators/temperature.glsl") {}
        std::string name() const override { return "Temperature"; }
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        std::vector<Output> outputs() const override
        {
            return {{"Temperature"}};
        }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerUInt("loBand", 0);
            settings->registerUInt("hiBand", 0);
            settings->registerFloat("falloff", 0.5f);
        }
    };

    REGISTER_OPERATOR(Temperature, Temperature::create);
}
