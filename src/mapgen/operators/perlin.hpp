#pragma once
#include <string>

#include <glm/glm.hpp>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class PerlinNoise : public BaseComputeShaderOp
    {
    public:
        static PerlinNoise *create()
        {
            return new PerlinNoise();
        }

        PerlinNoise() : BaseComputeShaderOp("src/mapgen/shaders/compute/perlin.glsl") {}

        std::string name() const override { return "PerlinNoise"; }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerFloat3("offset", glm::vec3(1));
            settings->registerInt("octaves", 8);
            settings->registerFloat("frequency", 0.003f);
            settings->registerFloat("amplitude", 1.0f);
            settings->registerFloat("lacunarity", 1.5f);
            settings->registerFloat("persistence", 0.66f);
        }
    };

    REGISTER_OPERATOR(PerlinNoise, PerlinNoise::create);
}
