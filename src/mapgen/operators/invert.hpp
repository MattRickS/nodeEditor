#pragma once
#include <string>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class InvertOp : public Operator
{
public:
    Shader shader;

    static InvertOp *create()
    {
        return new InvertOp();
    }

    InvertOp() : shader("src/mapgen/shaders/compute/invert.glsl") {}
    virtual std::string name() const { return "Invert"; }
    virtual std::vector<Layer> inLayers() const
    {
        return {LAYER_HEIGHTMAP};
    }
    virtual std::vector<Layer> outLayers() const
    {
        return {LAYER_HEIGHTMAP};
    }
    virtual bool process(RenderSet *renders)
    {
        // Setup shader
        shader.use();

        auto tex = renders->at(LAYER_HEIGHTMAP);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex->ID);
        glBindImageTexture(0, tex->ID, 0, GL_FALSE, 0, GL_READ_ONLY, tex->internalFormat());

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, outputs[0].ID);
        glBindImageTexture(1, outputs[0].ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, outputs[0].internalFormat());

        // Render
        glDispatchCompute(ceil(m_width / 8), ceil(m_height / 4), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        return true;
    }
    virtual void reset(){};
};

REGISTER_OPERATOR(InvertOp, InvertOp::create);
