#include "shader.h"

/*
Convenience class for drawing Fragment shaders without having to handle vertices
*/
class QuadShader : public Shader
{
public:
    QuadShader(const char *fragShader) : Shader("src/mapgen/shaders/quad.vs", fragShader) {}

    void setResolution(int width, int height)
    {
        use();
        setInt2("screenRes", width, height);
    }

    void draw()
    {
        use();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
};