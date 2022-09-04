#pragma once
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <glm/glm.hpp>

#include "Serializer.h"

typedef std::variant<bool, unsigned int, int, float, glm::vec2, glm::vec3, glm::vec4, glm::ivec2, std::string> SettingValue;
typedef std::map<std::string, SettingValue> SettingChoices;

const std::string &currentChoice(SettingChoices choices, SettingValue value);

enum SettingType
{
    SettingType_Bool,
    SettingType_Float,
    SettingType_Float2,
    SettingType_Float3,
    SettingType_Float4,
    SettingType_Int,
    SettingType_Int2,
    SettingType_UInt,
    SettingType_String
};

enum SettingHint
{
    SettingHint_None = 0,
    SettingHint_Channel = 1 << 0,     // Displays a channel selector. Supports Int
    SettingHint_Color = 1 << 1,       // Displays as a color. Supports Float3, Float4
    SettingHint_Logarithmic = 1 << 2, // UI interaction will make it easier to select smaller values. Supports Float
};

class Setting
{
public:
    Setting();
    Setting(const std::string name, const SettingType type, SettingValue value, SettingHint hints = SettingHint_None);
    Setting(const std::string name, const SettingType type, SettingValue value, SettingChoices choices);
    Setting(const std::string name, const SettingType type, SettingValue value, SettingValue min, SettingValue max, SettingHint hints = SettingHint_None);
    const std::string &name() const;
    SettingType type() const;
    SettingHint hints() const;

    void set(SettingValue value);
    bool isEdited() const;

    bool hasChoices() const;
    const SettingChoices &choices() const;
    const std::string &currentChoice() const;

    template <typename T>
    T defaultValue() const { return std::get<T>(m_defaultValue); }
    template <typename T>
    T value() const { return std::get<T>(m_value); }
    template <typename T>
    T &value() { return std::get<T>(m_value); }
    template <typename T>
    void setValue(T value) { m_value = value; }
    template <typename T>
    T min() const { return std::get<T>(m_min); }
    template <typename T>
    T max() const { return std::get<T>(m_max); }

private:
    std::string m_name;
    SettingType m_type;
    SettingValue m_defaultValue;
    SettingValue m_value;
    SettingHint m_hints = SettingHint_None;

    // XXX: Perhaps it would be better to just use void*...
    SettingValue m_min;
    SettingValue m_max;
    SettingChoices m_choices;
};

class Settings
{
public:
    typedef std::vector<Setting>::iterator iterator;
    typedef std::vector<Setting>::const_iterator const_iterator;

    iterator begin();
    iterator end();
    const_iterator cbegin() const;
    const_iterator cend() const;

    Setting *get(const std::string &key);
    const Setting *get(const std::string &key) const;

    // TODO: Add additional registration options and separate set methods
    void registerBool(const std::string &name, bool value, SettingHint hints = SettingHint_None);
    void registerBool(const std::string &name, bool value, SettingChoices choices);
    void registerUInt(const std::string &name, unsigned int value, unsigned int min = 0, unsigned int max = UINT_MAX, SettingHint hints = SettingHint_None);
    void registerUInt(const std::string &name, unsigned int value, SettingChoices choices);
    void registerInt(const std::string &name, int value, int min = INT_MIN, int max = INT_MAX, SettingHint hints = SettingHint_None);
    void registerInt(const std::string &name, int value, SettingChoices choices);
    void registerFloat(const std::string &name, float value, float min = 0.0f, float max = 1.0f, SettingHint hints = SettingHint_None);
    void registerFloat(const std::string &name, float value, SettingChoices choices);
    void registerFloat2(const std::string &name, glm::vec2 value, float min = 0.0f, float max = 1.0f, SettingHint hints = SettingHint_None);
    void registerFloat2(const std::string &name, glm::vec2 value, SettingChoices choices);
    void registerFloat3(const std::string &name, glm::vec3 value, float min = 0.0f, float max = 1.0f, SettingHint hints = SettingHint_None);
    void registerFloat3(const std::string &name, glm::vec3 value, SettingChoices choices);
    void registerFloat4(const std::string &name, glm::vec4 value, float min = 0.0f, float max = 1.0f, SettingHint hints = SettingHint_None);
    void registerFloat4(const std::string &name, glm::vec4 value, SettingChoices choices);
    void registerInt2(const std::string &name, glm::ivec2 value, SettingHint hints = SettingHint_None);
    void registerInt2(const std::string &name, glm::ivec2 value, SettingChoices choices);
    void registerString(const std::string &name, std::string value, SettingHint hints = SettingHint_None);
    void registerString(const std::string &name, std::string value, SettingChoices choices);

    bool getBool(const std::string &key) const;
    unsigned int getUInt(const std::string &key) const;
    int getInt(const std::string &key) const;
    float getFloat(const std::string &key) const;
    glm::vec2 getFloat2(const std::string &key) const;
    glm::vec3 getFloat3(const std::string &key) const;
    glm::vec4 getFloat4(const std::string &key) const;
    glm::ivec2 getInt2(const std::string &key) const;
    std::string getString(const std::string &key) const;

    bool serialize(Serializer *serializer, bool editedOnly = true) const;
    bool deserialize(Deserializer *deserializer);

protected:
    std::vector<Setting> m_settings;

    void validateUniqueSetting(const std::string &name) const;
};
