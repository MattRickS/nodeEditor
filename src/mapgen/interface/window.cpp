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
    window_->resize(width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    Window *window_ = (Window *)glfwGetWindowUserPointer(window);
    window_->onMouseMoved(xpos, ypos);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    Window *window_ = (Window *)glfwGetWindowUserPointer(window);
    window_->onMouseButtonChanged(button, action, mods);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    Window *window_ = (Window *)glfwGetWindowUserPointer(window);
    window_->onMouseScrolled(xoffset, yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Window *window_ = (Window *)glfwGetWindowUserPointer(window);
    window_->onKeyChanged(key, scancode, action, mods);
}

void close_callback(GLFWwindow *window)
{
    Window *window_ = (Window *)glfwGetWindowUserPointer(window);
    window_->onCloseRequested();
}

// =============================================================================
// Public

Window::Window(const char *name, unsigned int width, unsigned int height, const Context *sharedContext) : Context(name, width, height, sharedContext), m_width(width), m_height(height)
{
    if (!isInitialised())
    {
        return;
    }
    glfwSwapInterval(1); // Enable vsync
    connectSignals();
}

Window::~Window()
{
    disconnectSignals();
    if (m_window)
    {
        glfwDestroyWindow(m_window);
    }
}

void Window::display() { glfwSwapBuffers(m_window); }

void Window::close() { glfwSetWindowShouldClose(m_window, true); }
bool Window::isClosed() const { return glfwWindowShouldClose(m_window); }

void Window::resize(unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;
    sizeChanged.emit(m_width, m_height);
}

unsigned int Window::height() const { return m_height; }
unsigned int Window::width() const { return m_width; }

glm::vec2 Window::cursorPos() const
{
    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);
    return glm::vec2(xpos, ypos);
}

// =============================================================================
// Protected

void Window::connectSignals()
{
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
    glfwSetCursorPosCallback(m_window, mouse_callback);
    glfwSetMouseButtonCallback(m_window, mouse_button_callback);
    glfwSetScrollCallback(m_window, scroll_callback);
    glfwSetKeyCallback(m_window, key_callback);
    glfwSetWindowCloseCallback(m_window, close_callback);
}

void Window::disconnectSignals()
{
    cursorMoved.disconnect();
    mouseButtonChanged.disconnect();
    mouseScrolled.disconnect();
    keyChanged.disconnect();
    sizeChanged.disconnect();
}

bool Window::hasKeyPressed(int key) { return glfwGetKey(m_window, key) == GLFW_PRESS; }

// =============================================================================
// Callbacks

void Window::onMouseMoved(double xpos, double ypos) { cursorMoved.emit(xpos, ypos); }
void Window::onMouseButtonChanged(int button, int action, int mods) { mouseButtonChanged.emit(button, action, mods); }
void Window::onMouseScrolled(double xoffset, double yoffset) { mouseScrolled.emit(xoffset, yoffset); }
void Window::onKeyChanged(int key, int scancode, int action, int mods) { keyChanged.emit(key, scancode, action, mods); }
void Window::onWindowResized(int width, int height)
{
    resize(width, height);
    sizeChanged.emit(width, height);
}
void Window::onCloseRequested()
{
    // Reset the close state and propagate the decision to a Controller
    glfwSetWindowShouldClose(m_window, GLFW_FALSE);
    closeRequested.emit();
}
