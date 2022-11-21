#pragma once
#include "../gl/Context.hpp"
#include "../nodegraph/Scene.h"

class RenderScene : public Scene
{
public:
    RenderScene();

    const Context *context() const;
    glm::ivec2 defaultImageSize() const;
    void setDefaultImageSize(glm::ivec2 imageSize);

protected:
    Context m_context;
    GLuint m_quadVAO;

    virtual void process();
};
