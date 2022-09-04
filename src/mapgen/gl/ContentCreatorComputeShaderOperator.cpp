#include "../log.h"
#include "ContentCreatorComputeShaderOperator.h"

namespace Op
{
    ContentCreatorComputeShaderOperator::ContentCreatorComputeShaderOperator(const char *computeShader) : ComputeShaderOperator(computeShader) {}
    std::vector<OutputLayer> ContentCreatorComputeShaderOperator::outputLayers(const std::vector<RenderSetOperator const *> &inputs, [[maybe_unused]] Settings const *settings, Settings const *sceneSettings)
    {
        glm::ivec2 imageSize = settings->getInt2("imageSize");
        return {{DEFAULT_LAYER, (imageSize.x == 0 || imageSize.y == 0) ? outputLayerSize(0, inputs, sceneSettings) : imageSize}};
    }
    void ContentCreatorComputeShaderOperator::registerSettings(Settings *const settings) const
    {
        settings->registerInt2("imageSize", glm::ivec2(0));
    }
}
