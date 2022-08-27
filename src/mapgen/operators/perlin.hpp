#pragma once
#include <string>

#include <glm/glm.hpp>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class PerlinNoise : public ContentCreatorComputeShaderOp
    {
    public:
        static PerlinNoise *create()
        {
            return new PerlinNoise();
        }

        PerlinNoise() : ContentCreatorComputeShaderOp("src/mapgen/operators/perlin.glsl") {}

        std::string name() const override { return "PerlinNoise"; }
        void defaultSettings(Settings *const settings) const override
        {
            ContentCreatorComputeShaderOp::defaultSettings(settings);
            settings->registerFloat3("offset", glm::vec3(1), FLT_MIN, FLT_MAX);
            settings->registerInt("octaves", 8, 1, 16);
            settings->registerFloat("frequency", 0.003f, 0.0f, 1.0f, SettingHint_Logarithmic);
            settings->registerFloat("amplitude", 1.0f, 0.01f, 100.0f, SettingHint_Logarithmic);
            settings->registerFloat("lacunarity", 1.5f, 0.01f, 100.0f, SettingHint_Logarithmic);
            settings->registerFloat("persistence", 0.66f);
        }
    };

    REGISTER_OPERATOR(PerlinNoise, PerlinNoise::create);
}
