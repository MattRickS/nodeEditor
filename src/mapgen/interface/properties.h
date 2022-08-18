#pragma once
#include <string>

#include "../bounds.hpp"
#include "../nodegraph/node.h"
#include "../scene.h"
#include "../settings.h"
#include "panel.hpp"
#include "signal.hpp"

class Properties : public Panel
{
public:
    Signal<Node *, std::string, SettingValue> opSettingChanged;
    Signal<bool> pauseToggled;

    Properties(Bounds bounds);

    void setScene(Scene *scene);
    void setNode(NodeID node);

    void draw() override;

protected:
    Scene *m_scene = nullptr;
    NodeID m_nodeID;

    void drawNodeSettings(Node *node);
    void drawBoolSetting(Node *node, const Setting &setting);
    void drawFloatSetting(Node *node, const Setting &setting);
    void drawFloat2Setting(Node *node, const Setting &setting);
    void drawFloat3Setting(Node *node, const Setting &setting);
    void drawFloat4Setting(Node *node, const Setting &setting);
    void drawIntSetting(Node *node, const Setting &setting);
    void drawInt2Setting(Node *node, const Setting &setting);
    void drawUIntSetting(Node *node, const Setting &setting);
};