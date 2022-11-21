#pragma once
#include <string>
#include <vector>

#include "../gl/ContentCreatorComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    // TODO: Rename and use float4array, can be generic banding with optional separator
    class VectorBand : public ContentCreatorComputeShaderOperator
    {
    public:
        static VectorBand *create()
        {
            return new VectorBand();
        }

        VectorBand() : ContentCreatorComputeShaderOperator("src/mapgen/operators/VectorBand.glsl") {}
        void registerSettings(Settings *const settings) const override
        {
            ContentCreatorComputeShaderOperator::registerSettings(settings);
            settings->registerInt("divisionSize", 0, 0, 100);
            settings->registerFloat2Array("vectors", {{1, 1}, {0.5, 0.5}});
        }
    };

    REGISTER_OPERATOR(VectorBand, VectorBand::create);
}
