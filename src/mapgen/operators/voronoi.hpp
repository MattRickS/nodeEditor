#pragma once
#include <string>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class VoronoiNoise : public Operator
{
public:
    Shader shader;

    static VoronoiNoise *create()
    {
        return new VoronoiNoise();
    }

    VoronoiNoise() : shader("src/mapgen/shaders/compute/voronoi.glsl")
    {
        settings.registerInt2("offset", glm::ivec2(0));
        settings.registerFloat("size", 100.0f);
        settings.registerFloat("skew", 0.5f);
    }
    virtual std::string name() const { return "Voronoi"; }
    virtual std::vector<Layer> inLayers() const
    {
        return {};
    }
    virtual std::vector<Layer> outLayers() const
    {
        return {LAYER_HEIGHTMAP};
    }
    virtual bool process(RenderSet *renders)
    {
        shader.use();
        shader.setIVec2("offset", settings.getInt2("offset"));
        shader.setFloat("size", settings.getFloat("size"));
        shader.setFloat("skew", settings.getFloat("skew"));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, outputs[0].ID);
        glBindImageTexture(0, outputs[0].ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, outputs[0].internalFormat());

        glDispatchCompute(ceil(m_width / 8), ceil(m_height / 4), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        return true;
    }
    virtual void reset(){};
};

REGISTER_OPERATOR(VoronoiNoise, VoronoiNoise::create);
