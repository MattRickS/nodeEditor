#pragma once
#include <string>

#include "../operator.h"
#include "../renders.h"
#include "../settings.h"
#include "../shader.h"

class VoronoiNoise : public Operator
{
public:
    Shader shader;

    static VoronoiNoise *create()
    {
        return new VoronoiNoise();
    }

    VoronoiNoise() : shader("src/mapgen/shaders/compute/voronoi.glsl") {}
    std::string name() const override { return "Voronoi"; }
    void defaultSettings(Settings *settings) const override
    {
        settings->registerInt2("offset", glm::ivec2(0));
        settings->registerFloat("size", 100.0f);
        settings->registerFloat("skew", 0.5f);
    }
    bool process([[maybe_unused]] const std::vector<Texture *> &inputs, const std::vector<Texture *> &outputs, const Settings *settings) override
    {
        shader.use();
        shader.setIVec2("offset", settings->getInt2("offset"));
        shader.setFloat("size", settings->getFloat("size"));
        shader.setFloat("skew", settings->getFloat("skew"));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, outputs[0]->ID);
        glBindImageTexture(0, outputs[0]->ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, outputs[0]->internalFormat());

        glDispatchCompute(ceil(outputs[0]->width / 8), ceil(outputs[0]->height / 4), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        return true;
    }
};

REGISTER_OPERATOR(VoronoiNoise, VoronoiNoise::create);
