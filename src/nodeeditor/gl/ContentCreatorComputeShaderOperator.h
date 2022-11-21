#pragma once
#include <vector>

#include <glm/glm.hpp>

#include "../nodegraph/Settings.h"
#include "ComputeShaderOperator.h"

namespace Op
{
    /*
    Extends the ComputeShaderOperator to add an image size setting and default outputLayer implementation
    */
    class ContentCreatorComputeShaderOperator : public ComputeShaderOperator
    {
    public:
        ContentCreatorComputeShaderOperator(const char *computeShader);
        virtual std::vector<OutputLayer> outputLayers(const std::vector<RenderSetOperator const *> &inputs, Settings const *settings, Settings const *sceneSettings);
        virtual void registerSettings(Settings *const settings) const;
    };
}
