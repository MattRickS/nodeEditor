#pragma once
#include <string>

#include <glm/glm.hpp>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class PerlinNoiseOperator : public Operator
{
public:
    Shader shader;

    PerlinNoiseOperator();

    virtual OpType type() const;
    virtual std::string name() const;

    virtual std::vector<Layer> inLayers() const;
    virtual std::vector<Layer> outLayers() const;
    virtual bool process(RenderSet *renders);
    virtual void reset();
};
