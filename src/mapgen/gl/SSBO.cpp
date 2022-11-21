#include <GL/glew.h>

#include "../nodegraph/Settings.h"
#include "Shader.h"

#include "SSBO.h"

SSBO::SSBO()
{
    glGenBuffers(1, &m_id);
}
SSBO::~SSBO()
{
    glDeleteBuffers(1, &m_id);
}
bool SSBO::load(const Setting &setting, int binding, GLenum usage)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_id);
    if (setting.type() == SettingType_Float2Array)
    {
        std::vector<glm::vec2> value = setting.value<std::vector<glm::vec2>>();
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * value.size() * 2, value.data(), usage);
        return true;
    }
    else if (setting.type() == SettingType_Float4Array)
    {
        std::vector<glm::vec4> value = setting.value<std::vector<glm::vec4>>();
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * value.size() * 4, value.data(), usage);
        return true;
    }
    // TODO: Other array types. Note, float3 may need to bind as vec4...
    return false;
}
