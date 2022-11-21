#pragma once
#include <string>
#include <vector>

#include "../log.h"
#include "../nodegraph/Settings.h"
#include "ComputeShaderOperator.h"

namespace Op
{
    /*
    Intended for multiple process iterations of the same image.

    The first iteration loads the first input's default layer as the input image,
    and the "ping" layer as output.
    The second iteration loads the "ping" layer as the input image, and the "pong"
    layer as output.
    Subsequent iterations alternate the two textures so that the current output
    becomes the next input and vice versa.

    Derived classes should implement process to return true once processing is
    complete. The base implementation always returns true to protect against
    infinite processing if the derived class does not override process.

    The following shader should be used as a base for this class

        #version 430 core
        layout(local_size_x = 8, local_size_y = 4) in;
        layout(rgba32f, binding=0) uniform image2D imgIn;
        layout(rgba32f, binding=1) uniform image2D imgOut;

        uniform int _iteration;
    */
    class PingPongOperator : public ComputeShaderOperator
    {
    public:
        const std::string pingLayer = "ping";
        const std::string pongLayer = "pong";

        PingPongOperator(const char *computeShader);
        virtual std::vector<Input> inputs() const override;
        virtual bool process(const std::vector<RenderSetOperator const *> &inputs, Settings const *settings, Settings const *sceneSettings) override;
        virtual void reset() override;
        int iteration() const;
        glm::ivec2 imageSize(const std::vector<RenderSetOperator const *> &inputs) const;
        const std::string &currentOutputLayer() const;
        // Copies the current iteration's output to an output texture for the given layer
        bool copyToLayer(const std::string &layer);
        // Deletes the ping pong layers from the output
        void deletePingPongLayers();

    protected:
        int m_iteration = 0;
    };
}
