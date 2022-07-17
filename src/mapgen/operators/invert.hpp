#pragma once

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class InvertOperator : public Operator
{
public:
    Shader shader;

    InvertOperator() : shader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/invert.fs")
    {
    }
    virtual std::vector<Layer> inLayers() const
    {
        return {LAYER_HEIGHTMAP};
    }
    virtual std::vector<Layer> outLayers() const
    {
        return {LAYER_HEIGHTMAP};
    }
    virtual void process(RenderSet *renders)
    {
        // Setup shader
        shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renders->GetLayer(LAYER_HEIGHTMAP)->ID);
        shader.setInt("inImage", 0);

        // Render
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
        glViewport(0, 0, m_width, m_height);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Put outputs into render set
        PopulateRenderSet(renders);
    }
};
