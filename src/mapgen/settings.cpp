#include <map>
#include <string>
#include <variant>

#include <glm/glm.hpp>

#include "settings.h"
#include "util.h"

const std::string EMPTY_STRING = "";

const std::string &currentChoice(SettingChoices choices, SettingValue value)
{
    for (auto it = choices.cbegin(); it != choices.cend(); ++it)
    {
        if (it->second == value)
        {
            return it->first;
        }
    }
    return EMPTY_STRING;
}

// =============================================================================
// Setting
Setting::Setting() {}
Setting::Setting(const std::string name, const SettingType type, SettingValue value, SettingHint hints) : m_name(name), m_type(type), m_defaultValue(value), m_value(value), m_hints(hints) {}
Setting::Setting(const std::string name, const SettingType type, SettingValue value, SettingChoices choices) : m_name(name), m_type(type), m_defaultValue(value), m_value(value), m_choices(choices) {}
Setting::Setting(const std::string name, const SettingType type, SettingValue value, SettingValue min, SettingValue max, SettingHint hints) : m_name(name), m_type(type), m_defaultValue(value), m_value(value), m_hints(hints), m_min(min), m_max(max) {}
const std::string &Setting::name() const { return m_name; }
SettingType Setting::type() const { return m_type; }
SettingHint Setting::hints() const { return m_hints; }

void Setting::set(SettingValue value) { m_value = value; }
bool Setting::isEdited() const
{
    return m_value != m_defaultValue;
}

bool Setting::hasChoices() const { return !m_choices.empty(); }
const SettingChoices &Setting::choices() const { return m_choices; }
const std::string &Setting::currentChoice() const
{
    return ::currentChoice(m_choices, m_value);
}

// =============================================================================
// Settings

void Settings::validateUniqueSetting(const std::string &name) const
{
    // Key can only be registered once
    for (const auto setting : m_settings)
    {
        if (setting.name() == name)
        {
            throw std::invalid_argument(name);
        }
    }
}

Settings::iterator Settings::begin() { return m_settings.begin(); }
Settings::iterator Settings::end() { return m_settings.end(); }
Settings::const_iterator Settings::cbegin() const { return m_settings.cbegin(); }
Settings::const_iterator Settings::cend() const { return m_settings.cend(); }

Setting *Settings::get(const std::string &key)
{
    for (auto &setting : m_settings)
    {
        if (setting.name() == key)
        {
            return &setting;
        }
    }
    return nullptr;
}
const Setting *Settings::get(const std::string &key) const
{
    for (const auto &setting : m_settings)
    {
        if (setting.name() == key)
        {
            return &setting;
        }
    }
    return nullptr;
}

void Settings::registerBool(const std::string &name, bool value, SettingHint hints)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Bool, value, hints);
}
void Settings::registerUInt(const std::string &name, unsigned int value, unsigned int min, unsigned int max, SettingHint hints)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_UInt, value, min, max, hints);
}
void Settings::registerInt(const std::string &name, int value, int min, int max, SettingHint hints)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Int, value, min, max, hints);
}
void Settings::registerFloat(const std::string &name, float value, float min, float max, SettingHint hints)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Float, value, min, max, hints);
}
void Settings::registerFloat2(const std::string &name, glm::vec2 value, float min, float max, SettingHint hints)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Float2, value, min, max, hints);
}
void Settings::registerFloat3(const std::string &name, glm::vec3 value, float min, float max, SettingHint hints)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Float3, value, min, max, hints);
}
void Settings::registerFloat4(const std::string &name, glm::vec4 value, float min, float max, SettingHint hints)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Float4, value, min, max, hints);
}
void Settings::registerInt2(const std::string &name, glm::ivec2 value, SettingHint hints)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Int2, value, hints);
}
void Settings::registerString(const std::string &name, std::string value, SettingHint hints)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_String, value, hints);
}

void Settings::registerBool(const std::string &name, bool value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Bool, value, choices);
}
void Settings::registerUInt(const std::string &name, unsigned int value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_UInt, value, choices);
}
void Settings::registerInt(const std::string &name, int value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Int, value, choices);
}
void Settings::registerFloat(const std::string &name, float value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Float, value, choices);
}
void Settings::registerFloat2(const std::string &name, glm::vec2 value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Float2, value, choices);
}
void Settings::registerFloat3(const std::string &name, glm::vec3 value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Float3, value, choices);
}
void Settings::registerFloat4(const std::string &name, glm::vec4 value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Float4, value, choices);
}
void Settings::registerInt2(const std::string &name, glm::ivec2 value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_Int2, value, choices);
}
void Settings::registerString(const std::string &name, std::string value, SettingChoices choices)
{
    validateUniqueSetting(name);
    m_settings.emplace_back(name, SettingType_String, value, choices);
}

bool Settings::getBool(const std::string &key) const
{
    return get(key)->value<bool>();
}
unsigned int Settings::getUInt(const std::string &key) const
{
    return get(key)->value<unsigned int>();
}
int Settings::getInt(const std::string &key) const
{
    return get(key)->value<int>();
}
float Settings::getFloat(const std::string &key) const
{
    return get(key)->value<float>();
}
glm::vec2 Settings::getFloat2(const std::string &key) const
{
    return get(key)->value<glm::vec2>();
}
glm::vec3 Settings::getFloat3(const std::string &key) const
{
    return get(key)->value<glm::vec3>();
}
glm::vec4 Settings::getFloat4(const std::string &key) const
{
    return get(key)->value<glm::vec4>();
}
glm::ivec2 Settings::getInt2(const std::string &key) const
{
    return get(key)->value<glm::ivec2>();
}
std::string Settings::getString(const std::string &key) const
{
    return get(key)->value<std::string>();
}

bool Settings::serialize(Serializer *serializer, bool editedOnly) const
{
    bool ok = true;
    for (auto it = cbegin(); it != cend(); ++it)
    {
        if (editedOnly && !it->isEdited())
        {
            continue;
        }

        switch (it->type())
        {
        case SettingType_Bool:
            ok = ok && serializer->writePropertyBool(it->name(), it->value<bool>());
            break;
        case SettingType_Float:
            ok = ok && serializer->writePropertyFloat(it->name(), it->value<float>());
            break;
        case SettingType_Float2:
            ok = ok && serializer->writePropertyFloat2(it->name(), it->value<glm::vec2>());
            break;
        case SettingType_Float3:
            ok = ok && serializer->writePropertyFloat3(it->name(), it->value<glm::vec3>());
            break;
        case SettingType_Float4:
            ok = ok && serializer->writePropertyFloat4(it->name(), it->value<glm::vec4>());
            break;
        case SettingType_Int:
            ok = ok && serializer->writePropertyInt(it->name(), it->value<int>());
            break;
        case SettingType_Int2:
            ok = ok && serializer->writePropertyInt2(it->name(), it->value<glm::ivec2>());
            break;
        case SettingType_UInt:
            ok = ok && serializer->writePropertyUInt(it->name(), it->value<unsigned int>());
            break;
        case SettingType_String:
            ok = ok && serializer->writePropertyString(it->name(), it->value<std::string>());
            break;
        }
    }
    return ok;
}

bool Settings::deserialize(Deserializer *deserializer)
{
    bool ok = true;
    std::string property;
    while (ok && deserializer->readProperty(property))
    {
        Setting *s = get(property);
        if (!s)
        {
            LOG_ERROR("Setting not registered: %s", property.c_str());
            continue;
        }

        SettingValue value;
        switch (s->type())
        {
        case SettingType_Bool:
            value = false; // Preset so `get` can access a reference
            ok = ok && deserializer->readBool(std::get<bool>(value));
            break;
        case SettingType_Float:
            value = 0.0f; // Preset so `get` can access a reference
            ok = ok && deserializer->readFloat(std::get<float>(value));
            break;
        case SettingType_Float2:
            value = glm::vec2(0); // Preset so `get` can access a reference
            ok = ok && deserializer->readFloat2(std::get<glm::vec2>(value));
            break;
        case SettingType_Float3:
            value = glm::vec3(0); // Preset so `get` can access a reference
            ok = ok && deserializer->readFloat3(std::get<glm::vec3>(value));
            break;
        case SettingType_Float4:
            value = glm::vec4(0); // Preset so `get` can access a reference
            ok = ok && deserializer->readFloat4(std::get<glm::vec4>(value));
            break;
        case SettingType_Int:
            value = 0; // Preset so `get` can access a reference
            ok = ok && deserializer->readInt(std::get<int>(value));
            break;
        case SettingType_Int2:
            value = glm::ivec2(0); // Preset so `get` can access a reference
            ok = ok && deserializer->readInt2(std::get<glm::ivec2>(value));
            break;
        case SettingType_UInt:
            value = 0; // Preset so `get` can access a reference
            ok = ok && deserializer->readUInt(std::get<unsigned int>(value));
            break;
        case SettingType_String:
            value = ""; // Preset so `get` can access a reference
            ok = ok && deserializer->readString(std::get<std::string>(value));
            break;
        }

        if (ok)
        {
            s->set(value);
        }
        else
        {
            LOG_ERROR("Failed to deserialize setting: %s", property.c_str());
        }
    }
    return ok;
}
