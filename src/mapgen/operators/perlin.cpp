#include <string>

#include <glm/glm.hpp>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"
#include "perlin.h"

PerlinNoiseShader::PerlinNoiseShader() : Shader("src/mapgen/shaders/posUV.vs", "src/mapgen/shaders/noise/perlin.fs") {}

float PerlinNoiseShader::Frequency() { return m_frequency; }
void PerlinNoiseShader::SetFrequency(float frequency)
{
    use();
    m_frequency = frequency;
    setFloat("frequency", m_frequency);
}

glm::ivec2 PerlinNoiseShader::Offset() { return m_offset; }
void PerlinNoiseShader::SetOffset(glm::ivec2 offset)
{
    use();
    m_offset = offset;
    setInt2("offset", m_offset.x, m_offset.y);
}

PerlinNoiseOperator::PerlinNoiseOperator() : shader() {}

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
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    glViewport(0, 0, m_width, m_height);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    PopulateRenderSet(renders);
}
