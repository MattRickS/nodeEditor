#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../context.hpp"
#include "signal.hpp"

class Window : public Context
{
public:
    Signal<double, double> cursorMoved;
    Signal<int, int, int> mouseButtonChanged;
    Signal<double, double> mouseScrolled;
    Signal<int, int, int, int> keyChanged;
    Signal<int, int> sizeChanged;
    Signal<> closeRequested;

    Window(const char *name, unsigned int width, unsigned int height, const Context *sharedContext = nullptr);
    ~Window();

    void display();

    void close();
    bool isClosed() const;

    void resize(unsigned int width, unsigned int height);
    unsigned int height() const;
    unsigned int width() const;

    glm::vec2 cursorPos() const;

    virtual void onMouseMoved(double xpos, double ypos);
    virtual void onMouseButtonChanged(int button, int action, int mods);
    virtual void onMouseScrolled(double xoffset, double yoffset);
    virtual void onKeyChanged(int key, int scancode, int action, int mods);
    virtual void onWindowResized(int width, int height);
    virtual void onCloseRequested();

protected:
    unsigned int m_width, m_height;

    void connectSignals();
    void disconnectSignals();
    bool hasKeyPressed(int key);
};
