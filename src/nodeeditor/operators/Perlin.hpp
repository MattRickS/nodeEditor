#pragma once
#include <string>

#include <glm/glm.hpp>

#include "../gl/ContentCreatorComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class PerlinNoise : public ContentCreatorComputeShaderOperator
    {
    public:
        static PerlinNoise *create()
        {
            return new PerlinNoise();
        }

        PerlinNoise() : ContentCreatorComputeShaderOperator("src/nodeeditor/operators/Perlin.glsl") {}

        void registerSettings(Settings *const settings) const override
        {
            ContentCreatorComputeShaderOperator::registerSettings(settings);
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
