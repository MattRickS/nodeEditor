#pragma once
#include <string>

#include "../gl/ContentCreatorComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class VoronoiNoise : public ContentCreatorComputeShaderOperator
    {
    public:
        static VoronoiNoise *create()
        {
            return new VoronoiNoise();
        }

        VoronoiNoise() : ContentCreatorComputeShaderOperator("src/nodeeditor/operators/Voronoi.glsl") {}
        void registerSettings(Settings *const settings) const override
        {
            ContentCreatorComputeShaderOperator::registerSettings(settings);
            settings->registerInt2("offset", glm::ivec2(0));
            settings->registerFloat("size", 100.0f, 1.0f, 2048.0f);
            settings->registerFloat("skew", 0.5f);
        }
    };

    REGISTER_OPERATOR(VoronoiNoise, VoronoiNoise::create);
}
