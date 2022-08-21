#include <map>
#include <string>
#include <variant>

#include <glm/glm.hpp>

#include "settings.h"

const std::string EMPTY_STRING = "";

// =============================================================================
// Setting
Setting::Setting() {}
Setting::Setting(const std::string name, const SettingType type, SettingValue value) : m_name(name), m_type(type), m_value(value) {}
Setting::Setting(const std::string name, const SettingType type, SettingValue value, SettingChoices choices) : m_name(name), m_type(type), m_value(value), m_choices(choices) {}
Setting::Setting(const std::string name, const SettingType type, SettingValue value, SettingValue min, SettingValue max) : m_name(name), m_type(type), m_value(value), m_min(min), m_max(max) {}
const std::string &Setting::name() const { return m_name; }
SettingType Setting::type() const { return m_type; }

void Setting::set(SettingValue value) { m_value = value; }

bool Setting::hasChoices() const { return !m_choices.empty(); }
const SettingChoices &Setting::choices() const { return m_choices; }
const std::string &Setting::currentChoice() const
{
    for (auto it = m_choices.cbegin(); it != m_choices.cend(); ++it)
    {
        if (it->second == m_value)
        {
            return it->first;
        }
    }
    return EMPTY_STRING;
}

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
Settings::const_value_iterator Settings::cbegin() const { return m_settings.cbegin(); }
Settings::const_value_iterator Settings::cend() const { return m_settings.cend(); }

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
    m_settings[name] = Setting(name, SettingType_Bool, value);
}
void Settings::registerUInt(const std::string name, unsigned int value, unsigned int min, unsigned int max)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_UInt, value, min, max);
}
void Settings::registerInt(const std::string name, int value, int min, int max)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Int, value, min, max);
}
void Settings::registerFloat(const std::string name, float value, float min, float max)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Float, value, min, max);
}
void Settings::registerFloat2(const std::string name, glm::vec2 value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Float2, value);
}
void Settings::registerFloat3(const std::string name, glm::vec3 value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Float3, value);
}
void Settings::registerFloat4(const std::string name, glm::vec4 value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Float4, value);
}
void Settings::registerInt2(const std::string name, glm::ivec2 value)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Int2, value);
}

void Settings::registerBool(const std::string name, bool value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Bool, value, choices);
}
void Settings::registerUInt(const std::string name, unsigned int value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_UInt, value, choices);
}
void Settings::registerInt(const std::string name, int value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Int, value, choices);
}
void Settings::registerFloat(const std::string name, float value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Float, value, choices);
}
void Settings::registerFloat2(const std::string name, glm::vec2 value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Float2, value, choices);
}
void Settings::registerFloat3(const std::string name, glm::vec3 value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Float3, value, choices);
}
void Settings::registerFloat4(const std::string name, glm::vec4 value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Float4, value, choices);
}
void Settings::registerInt2(const std::string name, glm::ivec2 value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings[name] = Setting(name, SettingType_Int2, value, choices);
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
