#pragma once
#include <string>
#include <vector>

#include "../operator.h"
#include "../renders.h"
#include "../settings.h"
#include "../shader.h"

class AddOp : public Operator
{
public:
    Shader shader;

    static AddOp *create()
    {
        return new AddOp();
    }

    AddOp() : shader("src/mapgen/shaders/compute/add.glsl") {}
    std::string name() const override { return "Add"; }
    std::vector<Input> inputs() const override
    {
        return {{}, {}};
    }
    bool process(const std::vector<Texture *> &inputs,
                 const std::vector<Texture *> &outputs,
                 [[maybe_unused]] const Settings *settings) override
    {
        // Setup shader
        shader.use();

        auto inTex1 = inputs[0];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inTex1->ID);
        glBindImageTexture(0, inTex1->ID, 0, GL_FALSE, 0, GL_READ_ONLY, inTex1->internalFormat());

        auto inTex2 = inputs[1];
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, inTex2->ID);
        glBindImageTexture(2, inTex2->ID, 0, GL_FALSE, 0, GL_READ_ONLY, inTex2->internalFormat());

        auto outTex = outputs[0];
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, outTex->ID);
        glBindImageTexture(1, outTex->ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, outTex->internalFormat());

        // Render
        glDispatchCompute(ceil(outTex->width / 8), ceil(outTex->height / 4), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        return true;
    }
};

REGISTER_OPERATOR(AddOp, AddOp::create);
