#include "../constants.h"
#include "util.h"
#include "RenderScene.h"

RenderScene::RenderScene() : m_context("Scene")
{
}

const Context *RenderScene::context() const { return &m_context; }
glm::ivec2 RenderScene::defaultImageSize() const { return m_settings.getInt2(SCENE_SETTING_IMAGE_SIZE); }
void RenderScene::setDefaultImageSize(glm::ivec2 imageSize)
{
    // TODO: Needs a way to check if any operators are affected by this...
    //       RenderSetOperator can calculate m_inputRenderSets and recalculateImageSize still
    m_settings.get(SCENE_SETTING_IMAGE_SIZE)->set(imageSize);
    // bool changed = false;
    // for (auto it = getCurrentGraph()->begin(); it != getCurrentGraph()->end(); ++it)
    // {
    //     if (it->recalculateImageSize(&m_settings))
    //     {
    //         it->setDirty(true);
    //         changed = true;
    //     }
    // }
    // if (changed)
    // {
    //     setDirty();
    // }
}

void RenderScene::process()
{
    m_context.use();
    makeQuad(&m_quadVAO);
    Scene::process();
}