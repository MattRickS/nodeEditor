#include "graphelement.h"

GraphElement::GraphElement() {}
GraphElement::GraphElement(Bounds bounds) : m_bounds(bounds) {}

void GraphElement::setSelectFlag(SelectFlag flag)
{
    m_selectState = SelectFlag(m_selectState | flag);
}
bool GraphElement::hasSelectFlag(SelectFlag flag) const
{
    return m_selectState & flag;
}
void GraphElement::clearSelectFlag(SelectFlag flag)
{
    m_selectState = SelectFlag(m_selectState & ~flag);
}
Bounds GraphElement::bounds() const
{
    return m_bounds;
}
void GraphElement::setPos(glm::vec2 pos)
{
    m_bounds.setPos(pos);
}
void GraphElement::setSize(glm::vec2 size)
{
    m_bounds.setSize(size);
}
void GraphElement::move(glm::vec2 offset)
{
    m_bounds.move(offset);
}