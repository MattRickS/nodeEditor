#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    enum MergeMode
    {
        MergeMode_Difference = 0,
        MergeMode_Max = 1,
        MergeMode_Min = 2,
        MergeMode_Over = 3,
    };

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
            // TODO: Needs a combo box for selecting mode
            settings->registerInt("mode", (int)MergeMode_Over);
            settings->registerFloat("blend", 1.0f);
            settings->registerBool("alphaMask", true);
        }
    };

    REGISTER_OPERATOR(Merge, Merge::create);
}
