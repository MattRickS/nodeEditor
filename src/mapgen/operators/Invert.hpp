#pragma once
#include <string>
#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Invert : public ComputeShaderOperator
    {
    public:
        static Invert *create()
        {
            return new Invert();
        }

        Invert() : ComputeShaderOperator("src/mapgen/operators/Invert.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
    };

    REGISTER_OPERATOR(Invert, Invert::create);
}
