#pragma once
#include <string>

#include <glm/glm.hpp>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class PerlinNoise : public Operator
{
public:
    Shader shader;

    static PerlinNoise *create()
    {
        return new PerlinNoise();
    }

    PerlinNoise() : shader("src/mapgen/shaders/compute/perlin.glsl") {}

    std::string name() const override { return "PerlinNoise"; }
    void defaultSettings(Settings *settings) const override
    {
        settings->registerFloat("frequency", 0.01f);
        settings->registerInt2("offset", glm::ivec2(0));
    }
    bool process([[maybe_unused]] const std::vector<Texture *> &inputs, const std::vector<Texture *> &outputs, const Settings *settings) override
    {
        shader.use();
        shader.setFloat("frequency", settings->getFloat("frequency"));
        shader.setIVec2("offset", settings->getInt2("offset"));

        auto outTex = outputs[0];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, outTex->ID);
        glBindImageTexture(0, outTex->ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, outTex->internalFormat());

        glDispatchCompute(ceil(outTex->width / 8), ceil(outTex->height / 4), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        return true;
    }
};

REGISTER_OPERATOR(PerlinNoise, PerlinNoise::create);
