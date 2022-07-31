#pragma once
#include <string>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class VoronoiNoiseOperator : public Operator
{
public:
    Shader shader;

    VoronoiNoiseOperator() : shader("src/mapgen/shaders/compute/voronoi.glsl")
    {
        settings.Register<glm::ivec2>("offset", glm::ivec2(0));
        settings.Register<float>("size", 100.0f);
        settings.Register<float>("skew", 0.5f);
    }
    virtual OpType type() const { return OP_VORONOI; }
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
        shader.setIVec2("offset", settings.Get<glm::ivec2>("offset"));
        shader.setFloat("size", settings.Get<float>("size"));
        shader.setFloat("skew", settings.Get<float>("skew"));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, outputs[0].ID);
        glBindImageTexture(0, outputs[0].ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, outputs[0].internalFormat());

        glDispatchCompute(ceil(m_width / 8), ceil(m_height / 4), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        return true;
    }
    virtual void reset(){};
};
