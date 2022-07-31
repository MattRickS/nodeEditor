#pragma once
#include <string>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class ComputeOperator : public Operator
{
public:
    Shader shader;

    ComputeOperator() : shader("src/mapgen/shaders/compute/color.glsl")
    {
    }
    virtual OpType type() const { return OP_COMPUTE; }
    virtual std::string name() const { return "Compute"; }
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
        for (size_t i = 0; i < outputs.size(); ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, outputs[i].ID);
            glBindImageTexture(i, outputs[i].ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        }

        glDispatchCompute(ceil(m_width / 8), ceil(m_height / 4), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        return true;
    }
    virtual void reset(){};
};
