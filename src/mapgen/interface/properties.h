#pragma once
#include <string>

#include "../bounds.hpp"
#include "../nodegraph/node.h"
#include "../scene.h"
#include "../settings.h"
#include "panel.hpp"
#include "signal.hpp"
#include "window.h"

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

    void setScene(Scene *scene);

    void draw() override;

protected:
    Scene *m_scene = nullptr;
    std::string m_saveLoadPath = "/tmp/scene.scene";

    void drawNodeSettings(Node *node);
    void drawBoolSetting(Node *node, const Setting &setting);
    void drawFloatSetting(Node *node, const Setting &setting);
    void drawFloat2Setting(Node *node, const Setting &setting);
    void drawFloat3Setting(Node *node, const Setting &setting);
    void drawFloat4Setting(Node *node, const Setting &setting);
    void drawIntSetting(Node *node, const Setting &setting);
    void drawInt2Setting(Node *node, const Setting &setting);
    void drawUIntSetting(Node *node, const Setting &setting);
    void drawStringSetting(Node *node, const Setting &setting);
    void drawSettingChoices(Node *node, const Setting &setting);
    void drawChoices(Node *node, const char *name, const SettingChoices &choices, const char *currChoice);
};
