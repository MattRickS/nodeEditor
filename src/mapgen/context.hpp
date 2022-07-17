#pragma once
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

/*
An OpenGL context, window visibility is optional.
A display is still required, in future this may use an EGL device for headless rendering.
*/
class Context
{
protected:
    GLFWwindow *m_window;
    bool m_glew_init = false;

public:
    Context(const char *name) : Context(name, 1, 1, false) {}
    Context(const char *name, unsigned int width, unsigned int height, bool visible = true)
    {
        if (!visible)
        {
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only

#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif

        m_window = glfwCreateWindow(width, height, name, NULL, NULL);
        if (m_window == NULL)
            return;

        glfwMakeContextCurrent(m_window);

        // GLEW init
        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            std::cerr << "Failed to initialise glew: " << glewGetErrorString(err) << std::endl;
            return;
        }
        m_glew_init = true;
    }
    ~Context()
    {
        if (m_window)
            glfwDestroyWindow(m_window);
    }
    bool IsInitialised() const { return bool(m_window) && m_glew_init; }
};
