#include <string>

#include <glm/glm.hpp>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"
#include "perlin.h"

PerlinNoiseOperator::PerlinNoiseOperator() : shader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/noise/perlin.fs")
{
    settings.Register<float>("frequency", 0.01f);
    settings.Register<glm::ivec2>("offset", glm::ivec2(0));
}

OpType PerlinNoiseOperator::type() const { return OP_TERRAIN_GEN; }

std::string PerlinNoiseOperator::name() const { return "Noise"; }

std::vector<Layer> PerlinNoiseOperator::inLayers() const
{
    return {};
}
std::vector<Layer> PerlinNoiseOperator::outLayers() const
{
    return {LAYER_HEIGHTMAP};
}
void PerlinNoiseOperator::process(RenderSet *renders)
{
    shader.use();
    shader.setFloat("frequency", settings.Get<float>("frequency"));
    shader.setIVec2("offset", settings.Get<glm::ivec2>("offset"));
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    glViewport(0, 0, m_width, m_height);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    PopulateRenderSet(renders);
    m_processed = true;
}
bool PerlinNoiseOperator::isProcessed() const
{
    return m_processed;
}
