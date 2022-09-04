#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../log.h"

/*
An OpenGL context, window visibility is optional.
A display is still required, in future this may use an EGL device for headless rendering.
*/
class Context
{
public:
    Context(const char *name, Context *sharedContext = nullptr) : Context(name, 1, 1, sharedContext, false) {}
    Context(const char *name, unsigned int width, unsigned int height, const Context *sharedContext = nullptr, bool visible = true)
    {
        glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only

#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif

        m_window = glfwCreateWindow(width, height, name, NULL, sharedContext ? sharedContext->m_window : NULL);
        if (m_window == NULL)
            return;

        glfwMakeContextCurrent(m_window);

        // GLEW init
        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            LOG_ERROR("Failed to initialise glew: %s", glewGetErrorString(err));
            return;
        }
        m_glew_init = true;
    }
    ~Context()
    {
        if (m_window)
            glfwDestroyWindow(m_window);
    }
    bool isInitialised() const { return bool(m_window) && m_glew_init; }
    void use() { glfwMakeContextCurrent(m_window); }

protected:
    GLFWwindow *m_window;
    bool m_glew_init = false;
};
