#include <string>

#include <glm/glm.hpp>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"
#include "perlin.h"

PerlinNoise::PerlinNoise() : shader("src/mapgen/shaders/compute/perlin.glsl")
{
    settings.registerFloat("frequency", 0.01f);
    settings.registerInt2("offset", glm::ivec2(0));
}

std::string PerlinNoise::name() const { return "Noise"; }

std::vector<Layer> PerlinNoise::inLayers() const
{
    return {};
}
std::vector<Layer> PerlinNoise::outLayers() const
{
    return {LAYER_HEIGHTMAP};
}
bool PerlinNoise::process(RenderSet *renders)
{
    shader.use();
    shader.setFloat("frequency", settings.getFloat("frequency"));
    shader.setIVec2("offset", settings.getInt2("offset"));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, outputs[0].ID);
    glBindImageTexture(0, outputs[0].ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, outputs[0].internalFormat());

    glDispatchCompute(ceil(m_width / 8), ceil(m_height / 4), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    return true;
}
void PerlinNoise::reset() {}

REGISTER_OPERATOR(PerlinNoise, PerlinNoise::create);
