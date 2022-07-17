#pragma once

#include <glm/glm.hpp>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class PerlinNoiseShader : public Shader
{
protected:
    float m_frequency = 0.01f;
    glm::ivec2 m_offset = glm::ivec2(0);

public:
    PerlinNoiseShader();

    float Frequency();
    void SetFrequency(float frequency);

    glm::ivec2 Offset();
    void SetOffset(glm::ivec2 offset);
};

class PerlinNoiseOperator : public Operator
{
public:
    PerlinNoiseShader shader;

    PerlinNoiseOperator();

    virtual std::vector<Layer> inLayers() const;
    virtual std::vector<Layer> outLayers() const;
    virtual void process(RenderSet *renders);
};
