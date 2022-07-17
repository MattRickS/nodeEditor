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

    Window(const char *name, unsigned int width, unsigned int height);
    ~Window();

    void Display();

    void Close();
    bool IsClosed() const;

    void Resize(unsigned int width, unsigned int height);
    unsigned int Height() const;
    unsigned int Width() const;

    glm::vec2 CursorPos() const;

    virtual void OnMouseMoved(double xpos, double ypos);
    virtual void OnMouseButtonChanged(int button, int action, int mods);
    virtual void OnMouseScrolled(double xoffset, double yoffset);
    virtual void OnKeyChanged(int key, int scancode, int action, int mods);
    virtual void OnWindowResized(int width, int height);
    virtual void OnCloseRequested();

protected:
    unsigned int m_width, m_height;

    void ConnectSignals();
    void DisconnectSignals();
    bool HasKeyPressed(int key);
};
