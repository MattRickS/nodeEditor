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

Window::Window(unsigned int width, unsigned int height, const char *name) : m_width(width), m_height(height)
{
    m_window = glfwCreateWindow(width, height, name, NULL, NULL);
    if (m_window == NULL)
        return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only

#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync

    // GLEW init
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "Failed to initialise glew: " << glewGetErrorString(err) << std::endl;
        return;
    }
    m_glew_init = true;

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
bool Window::IsInitialised() const { return bool(m_window) && m_glew_init; }

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
    return glm::vec2(xpos, ypos);
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

void Window::OnMouseMoved(double xpos, double ypos) { cursorMoved.emit(xpos, ypos); }
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
