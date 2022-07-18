#include <vector>

#include "operators/invert.hpp"
#include "operators/perlin.h"
#include "operator.h"
#include "renders.h"
#include "mapmaker.h"

MapMaker::MapMaker(unsigned int width, unsigned int height) : m_width(width), m_height(height)
{
    // TODO: Switch to smart pointers
    operators.push_back(new PerlinNoiseOperator);
    operators.push_back(new InvertOperator);
    for (auto op : operators)
    {
        op->init(m_width, m_height);
    }
}
MapMaker::~MapMaker()
{
    // for (size_t i = operators.size() - 1; i >= 0; --i)
    // {
    //     delete operators[i];
    // }
}

unsigned int MapMaker::Width() const { return m_width; }
unsigned int MapMaker::Height() const { return m_height; }

const RenderSet *const MapMaker::GetRenderSet() const
{
    return &renderSet;
}

bool MapMaker::IsProcessed(size_t index)
{
    // TODO: Where should this be tracked?
    // If Operators store it, they have to remember to set it when finished processing
    // Better for the framework to manage it, but a bit uglier code
    return false;
}
void MapMaker::ProcessTo(size_t index)
{
    renderSet.clear();
    for (size_t i = 0; i <= index; ++i)
    {
        if (operators[i]->isProcessed())
        {
            // Accrue available renders from alreayd processed operators
            operators[i]->PopulateRenderSet(&renderSet);
        }
        else
        {
            // TODO: This needs some sort of guarantee of completion. Perhaps a max
            // limit or indication of progress
            while (!operators[i]->isProcessed())
            {
                operators[i]->process(&renderSet);
            }
        }
    }
}
void MapMaker::ProcessAll()
{
    ProcessTo(operators.size() - 1);
}
