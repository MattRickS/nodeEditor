#pragma once
#include <string>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class ErosionOp : public Operator
{
protected:
    unsigned int m_iterations = 0;

public:
    Shader erosionShader;
    // Shader reductionShader;  TODO: Will be needed to add outflow back to heightmap

    static ErosionOp *create()
    {
        return new ErosionOp();
    }

    ErosionOp() : erosionShader("src/mapgen/shaders/compute/erosion.glsl") //, reductionShader("src/mapgen/shaders/compute/erosion.glsl")
    {
        settings.registerUInt("iterations", 1);
    }
    virtual std::string name() const { return "Erosion"; }
    virtual std::vector<Layer> inLayers() const
    {
        return {LAYER_HEIGHTMAP};
    }
    virtual std::vector<Layer> outLayers() const
    {
        // TODO: deposit / erosion / waterFlow (vector + quantity)
        return {LAYER_HEIGHTMAP, LAYER_OUTFLOW};
    }
    virtual void preprocess(RenderSet *renders)
    {
        // Copy the input textures into the outputs so they can be bound as read-write.
        auto heightptr = renders->at(LAYER_HEIGHTMAP);
        glCopyImageSubData(heightptr->ID, GL_TEXTURE_2D, 0, 0, 0, 0,
                           outputs[0].ID, GL_TEXTURE_2D, 0, 0, 0, 0, m_width, m_height, 1);
    }
    virtual bool process(RenderSet *renders)
    {
        ++m_iterations;
        erosionShader.use();

        // Bind the output for read and write. It should already contain data either from
        // the input render on first run (copied during preprocess), or from prior iterations.
        for (size_t i = 0; i < outputs.size(); ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, outputs[i].ID);
            glBindImageTexture(i, outputs[i].ID, 0, GL_FALSE, 0, GL_READ_WRITE, outputs[i].internalFormat());
        }

        glDispatchCompute(ceil(m_width / 8), ceil(m_height / 4), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        return m_iterations == settings.getUInt("iterations");
    }
    virtual void reset()
    {
        m_iterations = 0;
    };
};

REGISTER_OPERATOR(ErosionOp, ErosionOp::create);
