#pragma once
#include <string>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

namespace Op
{
    class Erosion : public Operator
    {
    public:
        Shader erosionShader;
        // Shader reductionShader;  TODO: Will be needed to add outflow back to heightmap

        static Erosion *create()
        {
            return new Erosion();
        }

        Erosion() : erosionShader("src/mapgen/operators/erosion.glsl") //, reductionShader("src/mapgen/shaders/compute/erosion.glsl")
        {
        }
        std::string name() const override { return "Erosion"; }
        void defaultSettings(Settings *settings) const override
        {
            settings->registerUInt("iterations", 1);
        }
        std::vector<Input> inputs() const override
        {
            return {{"height"}, {"water", false}, {"flow", false}, {"sediment", false}};
        }
        std::vector<Output> outputs() const override
        {
            // First output is default
            return {{}, {"water"}, {"flow"}, {"sediment"}};
        }
        void preprocess(const std::vector<Texture *> &inputs, const std::vector<Texture *> &outputs, const Settings *settings) override
        {
            // Copy the input textures into the outputs so they can be bound as read-write.
            auto heightptr = inputs[0];
            glCopyImageSubData(heightptr->id(), GL_TEXTURE_2D, 0, 0, 0, 0,
                               outputs[0]->id(), GL_TEXTURE_2D, 0, 0, 0, 0, heightptr->width(), heightptr->height(), 1);
        }
        bool process(const std::vector<Texture *> &inputs, const std::vector<Texture *> &outputs, const Settings *settings) override
        {
            ++m_iterations;
            erosionShader.use();

            // Bind the output for read and write. It should already contain data either from
            // the input render on first run (copied during preprocess), or from prior iterations.
            for (size_t i = 0; i < outputs.size(); ++i)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, outputs[i]->id());
                glBindImageTexture(i, outputs[i]->id(), 0, GL_FALSE, 0, GL_READ_WRITE, outputs[i]->internalFormat());
            }

            glDispatchCompute(ceil(outputs[0]->width() / 8), ceil(outputs[0]->height() / 4), 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            return m_iterations == settings->getUInt("iterations");
        }
        virtual void reset()
        {
            m_iterations = 0;
            Operator::reset();
        };

    protected:
        unsigned int m_iterations = 0;
    };

    REGISTER_OPERATOR(Erosion, Erosion::create);
}
