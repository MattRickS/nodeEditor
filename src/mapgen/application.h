#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "interface/panel.hpp"
#include "interface/viewport.h"
#include "interface/ui.h"
#include "nodegraph/graphelement.h"
#include "scene.h"
#include "settings.h"

class Application
{
public:
    Application(Scene *scene, UI *ui);
    void exec();
    void close();

protected:
    Scene *m_scene;
    UI *m_ui;
    PixelPreview m_pixelPreview;
    TextureReader m_textureReader;

    Panel *m_panningPanel = nullptr;
    glm::vec2 m_lastCursorPos;
    bool m_isDragging = false;

    // FPS limiting as vsync does not appear to be working
    double m_lastFrameTime = 0;
    double m_fpsLimit = 1.0 / 60.0;

    GLuint m_quadVAO_UI;
    const float m_camNear = 0.1f;
    const float m_camFar = 100.0f;

    // Signals
    void onChannelChanged(Channel channel);
    void onKeyChanged(int key, int scancode, int action, int mode);
    void onLayerChanged(std::string layerName);
    void onMouseButtonChanged(int button, int action, int mods);
    void onMouseMoved(double xpos, double ypos);
    void onMouseScrolled(double xoffset, double yoffset);
    void onResize(int width, int height);

    void setSelectedNode(Node *node);

    // Viewport
    const Texture *currentTexture() const;
    void togglePause(bool pause);
    void updatePixelPreview(double xpos, double ypos);
    void updateProjection();

    // Nodegraph
    bool elementContainsPos(GraphElement *el, glm::vec2 pos) const;
    GraphElement *getElementAtPos(glm::vec2 pos);
    Panel *panelAtPos(glm::vec2 pos);
    void setHoverState(GraphElement *el, glm::vec2 cursorPos) const;

    // Scene
    void createNode(glm::ivec2 screenPos, std::string nodeType);
    void deleteSelectedNode();
    void setViewNode(Node *node);
    void updateSetting(Node *node, std::string key, SettingValue value);
    void onNodeSizeChanged(Node *node, glm::ivec2 imageSize);
    void onSceneSizeChanged(glm::ivec2 defaultImageSize);
    void onNewSceneRequested();
    void onLoadRequested(const std::string &filepath);
    void onSaveRequested(const std::string &filepath);
};
