#pragma once

#include "../bounds.hpp"
#include "../constants.h"

class GraphElement
{
public:
    void setSelectFlag(SelectFlag flag);
    bool hasSelectFlag(SelectFlag flag) const;
    void clearSelectFlag(SelectFlag flag);
    // Returns a copy of the Bounds.
    virtual Bounds bounds() const;
    // Allows modifying the bounds of the object
    void setPos(glm::vec2 pos);
    void setSize(glm::vec2 size);
    void move(glm::vec2 offset);

protected:
    SelectFlag m_selectState = SelectFlag_None;
    Bounds m_bounds;
};
