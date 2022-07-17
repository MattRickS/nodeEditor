#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "window.h"

// =============================================================================
// Callbacks

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    Window *window_ = (Window *)glfwGetWindowUserPointer(window);
    window_->Resize(width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    Window *window_ = (Window *)glfwGetWindowUserPointer(window);
    window_->OnMouseMoved(xpos, ypos);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    Window *window_ = (Window *)glfwGetWindowUserPointer(window);
    window_->OnMouseButtonChanged(button, action, mods);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    Window *window_ = (Window *)glfwGetWindowUserPointer(window);
    window_->OnMouseScrolled(xoffset, yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Window *window_ = (Window *)glfwGetWindowUserPointer(window);
    window_->OnKeyChanged(key, scancode, action, mods);
}

void close_callback(GLFWwindow *window)
{
    Window *window_ = (Window *)glfwGetWindowUserPointer(window);
    window_->OnCloseRequested();
}

// =============================================================================
// Public

Window::Window(const char *name, unsigned int width, unsigned int height) : Context(name, width, height), m_width(width), m_height(height)
{
    if (!IsInitialised())
        return;
    glfwSwapInterval(1); // Enable vsync
    ConnectSignals();
}

Window::~Window()
{
    DisconnectSignals();
    if (m_window)
        glfwDestroyWindow(m_window);
}

void Window::Display() { glfwSwapBuffers(m_window); }

void Window::Close() { glfwSetWindowShouldClose(m_window, true); }
bool Window::IsClosed() const { return glfwWindowShouldClose(m_window); }

void Window::Resize(unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;
    sizeChanged.emit(m_width, m_height);
}

unsigned int Window::Height() const { return m_height; }
unsigned int Window::Width() const { return m_width; }

glm::vec2 Window::CursorPos() const
{
    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);
    // Convert to use botleft as (0,0) instead of openGL's topleft
    return glm::vec2(xpos, m_height - ypos);
}

// =============================================================================
// Protected

void Window::ConnectSignals()
{
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
    glfwSetCursorPosCallback(m_window, mouse_callback);
    glfwSetMouseButtonCallback(m_window, mouse_button_callback);
    glfwSetScrollCallback(m_window, scroll_callback);
    glfwSetKeyCallback(m_window, key_callback);
    glfwSetWindowCloseCallback(m_window, close_callback);
}

void Window::DisconnectSignals()
{
    cursorMoved.disconnect();
    mouseButtonChanged.disconnect();
    mouseScrolled.disconnect();
    keyChanged.disconnect();
    sizeChanged.disconnect();
}

bool Window::HasKeyPressed(int key) { return glfwGetKey(m_window, key) == GLFW_PRESS; }

// =============================================================================
// Callbacks

// Mouse movement is converted to use bottom left as (0,0) (openGL uses top left)
void Window::OnMouseMoved(double xpos, double ypos) { cursorMoved.emit(xpos, m_height - ypos); }
void Window::OnMouseButtonChanged(int button, int action, int mods) { mouseButtonChanged.emit(button, action, mods); }
void Window::OnMouseScrolled(double xoffset, double yoffset) { mouseScrolled.emit(xoffset, yoffset); }
void Window::OnKeyChanged(int key, int scancode, int action, int mods) { keyChanged.emit(key, scancode, action, mods); }
void Window::OnWindowResized(int width, int height)
{
    Resize(width, height);
    sizeChanged.emit(width, height);
}
void Window::OnCloseRequested()
{
    // Reset the close state and propagate the decision to a Controller
    glfwSetWindowShouldClose(m_window, GLFW_FALSE);
    closeRequested.emit();
}
