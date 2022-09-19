#pragma once
#include <string>

#include "../Bounds.hpp"
#include "../gl/RenderScene.h"
#include "../nodegraph/Connector.h"
#include "../nodegraph/Node.h"
#include "../nodegraph/Settings.h"
#include "Panel.hpp"
#include "Signal.hpp"
#include "Window.h"

class Properties : public Panel
{
public:
    Signal<Node *, std::string, SettingValue> opSettingChanged;
    Signal<glm::ivec2> sceneSizeChanged; // TODO: Possibly should be global settings
    Signal<bool> pauseToggled;
    Signal<> newSceneRequested;
    Signal<const std::string &> saveRequested;
    Signal<const std::string &> loadRequested;

    Properties(Window *window, Bounds bounds);

    void setScene(RenderScene *scene);

    void draw() override;

protected:
    RenderScene *m_scene = nullptr;
    std::string m_saveLoadPath = "/home/mshaw/git/mapGen/.scratch/scenes/scene.scene";

    void drawGlobalProperties();

    void drawNodeSettings(Node *node);
    void drawBoolSetting(Node *node, const Setting &setting);
    void drawFloatSetting(Node *node, const Setting &setting);
    void drawFloat2Setting(Node *node, const Setting &setting);
    void drawFloat3Setting(Node *node, const Setting &setting);
    void drawFloat4Setting(Node *node, const Setting &setting);
    void drawFloat2ArraySetting(Node *node, const Setting &setting);
    void drawIntSetting(Node *node, const Setting &setting);
    void drawInt2Setting(Node *node, const Setting &setting);
    void drawUIntSetting(Node *node, const Setting &setting);
    void drawStringSetting(Node *node, const Setting &setting);
    void drawSettingChoices(Node *node, const Setting &setting);
    void drawChoices(Node *node, const char *name, const SettingChoices &choices, const char *currChoice);
};
