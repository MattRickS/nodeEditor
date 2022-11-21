#include <GL/glew.h>

#include "../nodegraph/Settings.h"
#include "Shader.h"

class SSBO
{
public:
    SSBO();
    ~SSBO();
    bool load(const Setting &setting, int binding, GLenum usage = GL_STATIC_DRAW);

protected:
    GLuint m_id;
};