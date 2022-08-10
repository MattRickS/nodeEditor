#pragma once
#include <glm/glm.hpp>

struct Bounds
{
    glm::vec2 min;
    glm::vec2 max;

    Bounds() : min(0), max(0) {}
    Bounds(glm::vec2 min, glm::vec2 max) : min(min), max(max) {}
    Bounds(float minx, float miny, float maxx, float maxy) : min(minx, miny), max(maxx, maxy) {}

    glm::vec2 pos() const { return min; }
    glm::vec2 center() const { return min + (max - min) * 0.5f; }
    glm::vec2 size() const { return max - min; }
    bool contains(glm::vec2 pos) const { return min.x <= pos.x && pos.x <= max.x && min.y <= pos.y && pos.y <= max.y; }

    void setPos(glm::vec2 pos)
    {
        glm::vec2 offset = pos - min;
        min = pos;
        max += offset;
    }
    void setSize(glm::vec2 size) { max = min + size; }
};
