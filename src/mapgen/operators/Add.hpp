#pragma once
#include <string>
#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Add : public ComputeShaderOperator
    {
    public:
        static Add *create()
        {
            return new Add();
        }

        Add() : ComputeShaderOperator("src/mapgen/operators/Add.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerBool("red", true);
            settings->registerBool("green", true);
            settings->registerBool("blue", true);
            settings->registerBool("alpha", false);
            settings->registerFloat("add", 0.0f);
        }
    };

    REGISTER_OPERATOR(Add, Add::create);
}
