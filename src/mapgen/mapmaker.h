#pragma once
#include <vector>

#include "context.hpp"
#include "operators/invert.hpp"
#include "operators/perlin.h"
#include "operator.h"
#include "renders.h"

class MapMaker
{
protected:
    unsigned int m_width;
    unsigned int m_height;
    // MapMaker owns no textures, each operator owns the textures it generates.
    // MapMaker instead owns a running mapping of Layer: Texture* as a RenderSet
    // that it passes as input to each operator. If rewinding to a previous operator,
    // this must be reset and re-populated by each prior operator in sequence.
    RenderSet renderSet;

public:
    Context context;
    // Making this public for now, should really expose an iterator of some sort
    std::vector<Operator *> operators;

    MapMaker(unsigned int width, unsigned int height);
    ~MapMaker();

    unsigned int Width() const;
    unsigned int Height() const;
    const RenderSet *const GetRenderSet() const;
    bool IsProcessed(size_t index);
    void ProcessTo(size_t index);
    void ProcessAll();
};