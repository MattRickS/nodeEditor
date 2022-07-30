#pragma once
#include <string>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class VoronoiNoiseOperator : public Operator
{
public:
    Shader shader;

    VoronoiNoiseOperator() : shader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/noise/voronoi.fs")
    {
        settings.Register<glm::ivec2>("offset", glm::ivec2(0));
        settings.Register<float>("size", 100.0f);
        settings.Register<float>("skew", 0.5f);
        settings.Register<float>("blend", 0.0f);
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
        shader.setFloat("blend", settings.Get<float>("blend"));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
        glViewport(0, 0, m_width, m_height);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        return true;
    }
    virtual void reset(){};
};
