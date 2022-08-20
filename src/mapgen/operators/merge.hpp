#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class Merge : public BaseComputeShaderOp
    {
    public:
        static Merge *create()
        {
            return new Merge();
        }

        Merge() : BaseComputeShaderOp("src/mapgen/operators/merge.glsl") {}
        std::string name() const override { return "Merge"; }
        std::vector<Input> inputs() const override
        {
            return {{"A"}, {"B"}, {"Mask", false}};
        }
        void defaultSettings(Settings *settings) const override
        {
            // Needs a combo box for mode, eg, over, divide, etc...
            // settings->registerInt("mode", 0);
            settings->registerFloat("blend", 1.0f);
        }
    };

    REGISTER_OPERATOR(Merge, Merge::create);
}
