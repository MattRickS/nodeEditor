#pragma once
#include <vector>

#include "../gl/ComputeShaderOperator.h"
#include "../gl/ConvolveKernel.h"
#include "../nodegraph/Settings.h"

namespace Op
{
    class ConvolveOperator : public ComputeShaderOperator
    {
    public:
        ConvolveOperator();
        virtual ~ConvolveOperator();

        virtual std::vector<Input> inputs() const override;
        virtual bool process(const std::vector<RenderSetOperator const *> &inputs,
                             Settings const *settings,
                             Settings const *sceneSettings) override;

        virtual void registerSettings(Settings *const settings) const override;
        virtual bool populateKernel(ConvolveKernel *kernel,
                                    const std::vector<RenderSetOperator const *> &inputs,
                                    Settings const *settings,
                                    Settings const *sceneSettings) = 0;

    protected:
        GLuint m_ssbo;
        ConvolveKernel m_kernel;
    };

}
