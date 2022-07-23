#pragma once
#include <string>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class InvertOperator : public Operator
{
protected:
    bool m_processed = false;

public:
    Shader shader;

    InvertOperator() : shader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/invert.fs")
    {
    }
    virtual OpType type() const { return OP_INVERT; }
    virtual std::string name() const { return "Invert"; }
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
        glBindTexture(GL_TEXTURE_2D, renders->at(LAYER_HEIGHTMAP)->ID);
        shader.setInt("inImage", 0);

        // Render
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
        glViewport(0, 0, m_width, m_height);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        m_processed = true;

        // Put outputs into render set
        PopulateRenderSet(renders);
    }
    virtual bool isProcessed() const
    {
        return m_processed;
    }
};
