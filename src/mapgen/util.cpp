#include <cstring>

#include <GL/glew.h>

#include "constants.h"
#include "util.h"

void makeQuad(GLuint *VAO)
{
    static const GLfloat vertexData[] = {
        // positions          // texture coords
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    glGenVertexArrays(1, VAO);
    glBindVertexArray(*VAO);

    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    GLuint quadEBO;
    glGenBuffers(1, &quadEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

const char *getChannelName(Channel channel)
{
    switch (channel)
    {
    case Channel_All:
        return "RGBA";
    case Channel_Red:
        return "Red";
    case Channel_Green:
        return "Green";
    case Channel_Blue:
        return "Blue";
    case Channel_Alpha:
        return "Alpha";
    default:
        return "";
    }
}

bool containsTextCaseInsensitive(const char *text, const char *search)
{
    size_t i1 = 0;
    while (text[i1] != '\0')
    {
        size_t i2 = 0;
        while (search[i2] != '\0')
        {
            // Search does not fit in the rest of the text
            if (text[i1 + i2] == '\0')
            {
                return false;
            }
            // Break on the first mismatching character
            if (std::tolower(text[i1 + i2]) != std::tolower(search[i2]))
            {
                break;
            }
            // Keep incrementing search index as long as they match
            ++i2;
        }
        // The whole search string was matched successfully
        if (search[i2] == '\0')
        {
            return true;
        }
        ++i1;
    }

    return false;
}
