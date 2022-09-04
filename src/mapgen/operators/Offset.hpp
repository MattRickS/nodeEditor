#pragma once
#include <string>
#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class Offset : public ComputeShaderOperator
    {
    public:
        static Offset *create()
        {
            return new Offset();
        }

        Offset() : ComputeShaderOperator("src/mapgen/operators/Offset.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerInt2("offset", glm::ivec2(0));
        }
    };

    REGISTER_OPERATOR(Offset, Offset::create);
}
