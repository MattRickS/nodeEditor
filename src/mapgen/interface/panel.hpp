#pragma once

#include <glm/glm.hpp>

class Panel
{
public:
    Panel(glm::ivec2 pos, glm::ivec2 size) : m_pos(pos), m_size(size) {}
    virtual ~Panel() = default;

    glm::ivec2 pos() const { return m_pos; }
    glm::ivec2 size() const { return m_size; }
    void setPos(glm::ivec2 pos) { m_pos = pos; }
    void setSize(glm::ivec2 size) { m_size = size; }

    virtual void draw() = 0;

protected:
    glm::ivec2 m_pos;
    glm::ivec2 m_size;
};
