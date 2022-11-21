#pragma once
#include <vector>

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "../nodegraph/Settings.h"
#include "RenderSetOperator.h"
#include "Shader.h"
#include "SSBO.h"

namespace Op
{
    struct OutputLayer
    {
        const std::string layer;
        const glm::ivec2 imageSize;
    };

    /*
    Automatically binds all settings and images to a compute shader following an explicit convention.

    - local_size_x = 8, local_size_y = 4, local_size_z = 1.
    - All inputs/outputs are rgab32f image2D uniforms.
    - All registerSettings will be set as either
        - uniforms using the same name and type.
        - SSBO bindings for dynamic array types (eg, Float2Array) using the std430 layout.
    - The default layer of each input's renderset is bound in sequential order starting from layout binding 0.
    - The outputs() method defines outputs and their expected size. These are bound sequentially from the next available binding after inputs.
    - Optional inputs should define an additional uniform _ignoreImageN where N is the input index. This will be set to true if the input image exists, or false if not.

    For example, a ComputeShaderOperator defined as:

        virtual std::vector<Input> inputs() const
        {
            return {{"Left"}, {"Right", false}};
        }
        virtual std::vector<OutputLayer> outputLayers(const std::vector<Operator const *> &inputs, Settings const *settings, Settings const *sceneSettings)
        {
            return {{"layer1", {100, 100}}, {"layer2", 100, 100}};
        }
        virtual void registerSettings(Settings const * settings) const
        {
            settings->registerInt("num", 10);
            settings->registerFloat2Array("vectors", {{1, 1}, {0, 0}});
        }

    ...would automatically map to a glsl shader defined as:

        #version 430 core
        layout(local_size_x = 8, local_size_y = 4) in;
        layout(rgba32f, binding=0) uniform image2D imgIn0;
        layout(rgba32f, binding=1) uniform image2D imgIn1;
        layout(rgba32f, binding=2) uniform image2D imgOut0;
        layout(rgba32f, binding=3) uniform image2D imgOut1;
        layout(std430, binding=0) buffer vectorsBlock
        {
            data[];
        } vectors;

        uniform int num;
        uniform bool _ignoreImage1;

    Note that for SSBOs the block/instance name are not fixed, only the binding which is
    incremented from 0 for each setting in order.
    */
    class ComputeShaderOperator : public RenderSetOperator
    {
    public:
        ComputeShaderOperator(const char *computeShader);
        virtual std::vector<OutputLayer> outputLayers(const std::vector<RenderSetOperator const *> &inputs, Settings const *settings, Settings const *sceneSettings);
        virtual bool process(const std::vector<RenderSetOperator const *> &inputs, Settings const *settings, Settings const *sceneSettings);

    protected:
        Shader m_shader;
        std::vector<SSBO> m_ssbos;

        void render(glm::ivec2 imageSize);
        void bindSSBO(size_t index, const Setting &setting);
    };
}
