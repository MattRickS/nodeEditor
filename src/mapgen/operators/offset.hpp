#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../settings.h"

namespace Op
{
    class Offset : public BaseComputeShaderOp
    {
    public:
        static Offset *create()
        {
            return new Offset();
        }

        Offset() : BaseComputeShaderOp("src/mapgen/operators/offset.glsl") {}
        std::string name() const override { return "Offset"; }
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void defaultSettings(Settings *const settings) const override
        {
            settings->registerInt2("offset", glm::ivec2(0));
        }
    };

    REGISTER_OPERATOR(Offset, Offset::create);
}
