#pragma once
#include <string>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class VoronoiNoise : public BaseComputeShaderOp
    {
    public:
        static VoronoiNoise *create()
        {
            return new VoronoiNoise();
        }

        VoronoiNoise() : BaseComputeShaderOp("src/mapgen/operators/voronoi.glsl") {}
        std::string name() const override { return "Voronoi"; }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerInt2("offset", glm::ivec2(0));
            settings->registerFloat("size", 100.0f);
            settings->registerFloat("skew", 0.5f);
        }
    };

    REGISTER_OPERATOR(VoronoiNoise, VoronoiNoise::create);
}
