#pragma once
#include <string>
#include <vector>

#include "../gl/PingPongOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    class JumpFlood : public PingPongOperator
    {
    public:
        const std::string pixelLayer = "Pixel";

        static JumpFlood *create()
        {
            return new JumpFlood();
        }

        JumpFlood() : PingPongOperator("src/nodeeditor/operators/JumpFlood.glsl") {}
        bool process(const std::vector<RenderSetOperator const *> &inputs, Settings const *settings, Settings const *sceneSettings) override
        {
            glm::ivec2 size = imageSize(inputs);
            int offset = std::max(size.x, size.y) / 2;
            for (int i = 0; i < iteration(); ++i)
            {
                offset /= 2;
            }

            LOG_DEBUG("Jump Flood offset: %d", offset)
            m_shader.use();
            m_shader.setInt("offset", offset);

            bool ok = PingPongOperator::process(inputs, settings, sceneSettings);

            // The algorithm is only finished after immediate neighbours have been processed.
            ok = ok && (offset == 1);

            if (ok)
            {
                copyToLayer(pixelLayer);
                deletePingPongLayers();
            }

            return ok;
        }
    };

    REGISTER_OPERATOR(JumpFlood, JumpFlood::create);
}
