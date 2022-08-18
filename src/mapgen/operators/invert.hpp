#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class Invert : public BaseComputeShaderOp
    {
    public:
        static Invert *create()
        {
            return new Invert();
        }

        Invert() : BaseComputeShaderOp("src/mapgen/operators/invert.glsl") {}
        std::string name() const override { return "Invert"; }
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
    };

    REGISTER_OPERATOR(Invert, Invert::create);
}
