#pragma once

#include <glm/glm.hpp>

#include "bounds.hpp"

class Panel
{
public:
    Panel(Bounds bounds) : m_bounds(bounds) {}
    virtual ~Panel() = default;

    const Bounds &bounds() const { return m_bounds; }
    glm::ivec2 pos() const { return m_bounds.min; }
    glm::ivec2 size() const { return m_bounds.size(); }
    void setPos(glm::ivec2 pos) { m_bounds.min = pos; }
    void setSize(glm::ivec2 size) { m_bounds.max = m_bounds.min + glm::vec2(size); }

    virtual void draw() = 0;

protected:
    Bounds m_bounds;
};
