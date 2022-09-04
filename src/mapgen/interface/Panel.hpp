#pragma once

#include <glm/glm.hpp>

#include "../Bounds.hpp"
#include "Window.h"

class Panel
{
public:
    Panel(Window *window, Bounds bounds) : m_window(window), m_bounds(bounds) {}
    virtual ~Panel() = default;

    const Bounds &bounds() const { return m_bounds; }
    glm::ivec2 pos() const { return m_bounds.pos(); }
    glm::ivec2 size() const { return m_bounds.size(); }
    void setPos(glm::ivec2 pos) { m_bounds.setPos(pos); }
    void setSize(glm::ivec2 size) { m_bounds.setSize(glm::vec2(size)); }
    void setBounds(Bounds bounds) { m_bounds = bounds; }

    virtual void draw() = 0;

protected:
    Window *m_window;
    Bounds m_bounds;
};
