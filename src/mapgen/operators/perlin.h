#pragma once
#include <string>

#include <glm/glm.hpp>

#include "../operator.h"
#include "../renders.h"
#include "../shader.h"

class PerlinNoise : public Operator
{
public:
    Shader shader;

    static PerlinNoise *create()
    {
        return new PerlinNoise();
    }

    PerlinNoise();

    virtual std::string name() const;

    virtual std::vector<Layer> inLayers() const;
    virtual std::vector<Layer> outLayers() const;
    virtual bool process(RenderSet *renders);
    virtual void reset();
};
