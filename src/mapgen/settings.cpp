#include <map>
#include <string>
#include <variant>

#include <glm/glm.hpp>

#include "settings.h"

// =============================================================================
// Setting
Setting::Setting() {}
Setting::Setting(const std::string name, const SettingType type, SettingValue value) : m_name(name), m_type(type), m_value(value) {}
const std::string &Setting::name() const { return m_name; }
SettingType Setting::type() const { return m_type; }

template <typename T>
T Setting::value() const { return std::get<T>(m_value); }

template <typename T>
void Setting::setValue(T value) { m_value = value; }

void Setting::set(SettingValue value) { m_value = value; }

// =============================================================================
// Settings

void Settings::validateUniqueSetting(const std::string &name) const
{
    // Key can only be registered once
    if (m_settings.find(name) != m_settings.end())
    {
        throw std::invalid_argument(name);
    }
}
void Settings::validateKeyExists(const std::string &name) const
{
    if (m_settings.find(name) == m_settings.end())
    {
        throw std::out_of_range(name);
    }
}

Settings::value_iterator Settings::begin() { return m_settings.begin(); }
Settings::value_iterator Settings::end() { return m_settings.end(); }

Setting *Settings::get(const std::string key)
{
    if (m_settings.find(key) == m_settings.end())
    {
        throw std::out_of_range(key);
    }
    return &m_settings[key];
}
void Settings::registerBool(const std::string name, bool value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, S_BOOL, value);
}
void Settings::registerUInt(const std::string name, unsigned int value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, S_UINT, value);
}
void Settings::registerInt(const std::string name, int value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, S_INT, value);
}
void Settings::registerFloat(const std::string name, float value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, S_FLOAT, value);
}
void Settings::registerFloat2(const std::string name, glm::vec2 value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, S_FLOAT2, value);
}
void Settings::registerFloat3(const std::string name, glm::vec3 value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, S_FLOAT3, value);
}
void Settings::registerFloat4(const std::string name, glm::vec4 value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, S_FLOAT4, value);
}
void Settings::registerInt2(const std::string name, glm::ivec2 value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, S_INT2, value);
}
bool Settings::getBool(const std::string key) const
{
    validateKeyExists(key);
    return m_settings.at(key).value<bool>();
}
unsigned int Settings::getUInt(const std::string key) const
{
    validateKeyExists(key);
    return m_settings.at(key).value<unsigned int>();
}
int Settings::getInt(const std::string key) const
{
    validateKeyExists(key);
    return m_settings.at(key).value<int>();
}
float Settings::getFloat(const std::string key) const
{
    validateKeyExists(key);
    return m_settings.at(key).value<float>();
}
glm::vec2 Settings::getFloat2(const std::string key) const
{
    validateKeyExists(key);
    return m_settings.at(key).value<glm::vec2>();
}
glm::vec3 Settings::getFloat3(const std::string key) const
{
    validateKeyExists(key);
    return m_settings.at(key).value<glm::vec3>();
}
glm::vec4 Settings::getFloat4(const std::string key) const
{
    validateKeyExists(key);
    return m_settings.at(key).value<glm::vec4>();
}
glm::ivec2 Settings::getInt2(const std::string key) const
{
    validateKeyExists(key);
    return m_settings.at(key).value<glm::ivec2>();
}
