#pragma once
#include <string>
#include <vector>

#include "../constants.h"
#include "../gl/ComputeShaderOperator.h"
#include "../nodegraph/OperatorRegistry.hpp"
#include "../nodegraph/Settings.h"

namespace Op
{
    enum MergeMode
    {
        MergeMode_Atop = 0,
        MergeMode_Average = 1,
        MergeMode_ColorBurn = 2,
        MergeMode_ColorDodge = 3,
        MergeMode_ConjointOver = 4,
        MergeMode_Copy = 5,
        MergeMode_Difference = 6,
        MergeMode_DisjointOver = 7,
        MergeMode_Divide = 8,
        MergeMode_Exclusion = 9,
        MergeMode_From = 10,
        MergeMode_Geometric = 11,
        MergeMode_HardLight = 12,
        MergeMode_Hypot = 13,
        MergeMode_In = 14,
        MergeMode_Mask = 15,
        MergeMode_Matte = 16,
        MergeMode_Max = 17,
        MergeMode_Min = 18,
        MergeMode_Minus = 19,
        MergeMode_Multiply = 20,
        MergeMode_Out = 21,
        MergeMode_Over = 22,
        MergeMode_Overlay = 23,
        MergeMode_Plus = 24,
        MergeMode_Screen = 25,
        MergeMode_SoftLight = 26,
        MergeMode_Stencil = 27,
        MergeMode_Under = 28,
        MergeMode_Xor = 29
    };

    class Merge : public ComputeShaderOperator
    {
    public:
        static Merge *create()
        {
            return new Merge();
        }

        Merge() : ComputeShaderOperator("src/mapgen/operators/Merge.glsl") {}
        std::vector<Input> inputs() const override
        {
            return {{"A"}, {"B"}, {"Mask", false}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerInt("mode",
                                  (int)MergeMode_Over,
                                  {{"atop", MergeMode_Atop},
                                   {"average", MergeMode_Average},
                                   {"color-burn", MergeMode_ColorBurn},
                                   {"color-dodge", MergeMode_ColorDodge},
                                   {"conjoint-over", MergeMode_ConjointOver},
                                   {"copy", MergeMode_Copy},
                                   {"difference", MergeMode_Difference},
                                   {"disjoint-over", MergeMode_DisjointOver},
                                   {"divide", MergeMode_Divide},
                                   {"exclusion", MergeMode_Exclusion},
                                   {"from", MergeMode_From},
                                   {"geometric", MergeMode_Geometric},
                                   {"hard-light", MergeMode_HardLight},
                                   {"hypot", MergeMode_Hypot},
                                   {"in", MergeMode_In},
                                   {"mask", MergeMode_Mask},
                                   {"matte", MergeMode_Matte},
                                   {"max", MergeMode_Max},
                                   {"min", MergeMode_Min},
                                   {"minus", MergeMode_Minus},
                                   {"multiply", MergeMode_Multiply},
                                   {"out", MergeMode_Out},
                                   {"over", MergeMode_Over},
                                   {"overlay", MergeMode_Overlay},
                                   {"plus", MergeMode_Plus},
                                   {"screen", MergeMode_Screen},
                                   {"soft-light", MergeMode_SoftLight},
                                   {"stencil", MergeMode_Stencil},
                                   {"under", MergeMode_Under},
                                   {"xor", MergeMode_Xor}});
            settings->registerFloat("blend", 1.0f);
            settings->registerBool("alphaMask", true);
            settings->registerInt("maskChannel", ::Channel_Alpha, 0, 3, SettingHint_Channel);
        }
    };

    REGISTER_OPERATOR(Merge, Merge::create);
}
