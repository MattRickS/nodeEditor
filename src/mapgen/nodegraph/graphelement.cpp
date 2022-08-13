#include "graphelement.h"

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
    glm::vec2 offset = pos - m_bounds.min;
    m_bounds.min = pos;
    m_bounds.max += offset;
}
void GraphElement::setSize(glm::vec2 size)
{
    m_bounds.max = m_bounds.min + size;
}
void GraphElement::move(glm::vec2 offset)
{
    m_bounds.min += offset;
    m_bounds.max += offset;
}