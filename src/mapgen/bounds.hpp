#pragma once
#include <glm/glm.hpp>

class Bounds
{
public:
    Bounds() : m_min(0), m_max(0) {}
    Bounds(glm::vec2 min, glm::vec2 max) : m_min(min), m_max(max) {}
    Bounds(float minx, float miny, float maxx, float maxy) : m_min(minx, miny), m_max(maxx, maxy) {}

    glm::vec2 min() const { return m_min; }
    glm::vec2 max() const { return m_max; }
    glm::vec2 pos() const { return m_min; }
    glm::vec2 center() const { return m_min + (m_max - m_min) * 0.5f; }
    glm::vec2 size() const { return m_max - m_min; }
    bool contains(glm::vec2 pos) const { return m_min.x <= pos.x && pos.x <= m_max.x && m_min.y <= pos.y && pos.y <= m_max.y; }

    void setPos(glm::vec2 pos)
    {
        glm::vec2 offset = pos - m_min;
        m_min = pos;
        m_max += offset;
    }
    void setSize(glm::vec2 size) { m_max = m_min + size; }
    void move(glm::vec2 offset)
    {
        m_min += offset;
        m_max += offset;
    }

protected:
    glm::vec2 m_min;
    glm::vec2 m_max;
};
