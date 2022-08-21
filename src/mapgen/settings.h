#pragma once
#include <map>
#include <string>
#include <variant>

#include <glm/glm.hpp>

typedef std::variant<bool, unsigned int, int, float, glm::vec2, glm::vec3, glm::vec4, glm::ivec2> SettingValue;
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
};

enum SettingHint
{
    SettingHint_None,
    SettingHint_Channel,     // Displays a channel selector. Supports Int
    SettingHint_Color,       // Displays as a color. Supports Float3, Float4
    SettingHint_Logarithmic, // UI interaction will make it easier to select smaller values. Supports Float
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

    bool hasChoices() const;
    const SettingChoices &choices() const;
    const std::string &currentChoice() const;

    template <typename T>
    T value() const { return std::get<T>(m_value); }
    template <typename T>
    void setValue(T value) { m_value = value; }
    template <typename T>
    T min() const { return std::get<T>(m_min); }
    template <typename T>
    T max() const { return std::get<T>(m_max); }

private:
    std::string m_name;
    SettingType m_type;
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
    typedef std::map<std::string, Setting>::iterator pair_iterator;
    typedef std::map<std::string, Setting>::const_iterator const_pair_iterator;

    class value_iterator : public pair_iterator
    {
    public:
        value_iterator() : pair_iterator() {}
        value_iterator(pair_iterator it) : pair_iterator(it) {}
        Setting *operator->() { return &(pair_iterator::operator->()->second); }
        Setting &operator*() { return pair_iterator::operator*().second; }
    };
    class const_value_iterator : public const_pair_iterator
    {
    public:
        const_value_iterator() : const_pair_iterator() {}
        const_value_iterator(const_pair_iterator it) : const_pair_iterator(it) {}
        const Setting *operator->() { return &(const_pair_iterator::operator->()->second); }
        const Setting &operator*() { return const_pair_iterator::operator*().second; }
    };

    value_iterator begin();
    value_iterator end();
    const_value_iterator cbegin() const;
    const_value_iterator cend() const;

    Setting *get(const std::string key);

    // TODO: Add additional registration options and separate set methods
    void registerBool(const std::string name, bool value, SettingHint hints = SettingHint_None);
    void registerBool(const std::string name, bool value, SettingChoices choices);
    void registerUInt(const std::string name, unsigned int value, unsigned int min = 0, unsigned int max = UINT_MAX, SettingHint hints = SettingHint_None);
    void registerUInt(const std::string name, unsigned int value, SettingChoices choices);
    void registerInt(const std::string name, int value, int min = INT_MIN, int max = INT_MAX, SettingHint hints = SettingHint_None);
    void registerInt(const std::string name, int value, SettingChoices choices);
    void registerFloat(const std::string name, float value, float min = 0.0f, float max = 1.0f, SettingHint hints = SettingHint_None);
    void registerFloat(const std::string name, float value, SettingChoices choices);
    void registerFloat2(const std::string name, glm::vec2 value, SettingHint hints = SettingHint_None);
    void registerFloat2(const std::string name, glm::vec2 value, SettingChoices choices);
    void registerFloat3(const std::string name, glm::vec3 value, SettingHint hints = SettingHint_None);
    void registerFloat3(const std::string name, glm::vec3 value, SettingChoices choices);
    void registerFloat4(const std::string name, glm::vec4 value, SettingHint hints = SettingHint_None);
    void registerFloat4(const std::string name, glm::vec4 value, SettingChoices choices);
    void registerInt2(const std::string name, glm::ivec2 value, SettingHint hints = SettingHint_None);
    void registerInt2(const std::string name, glm::ivec2 value, SettingChoices choices);

    bool getBool(const std::string key) const;
    unsigned int getUInt(const std::string key) const;
    int getInt(const std::string key) const;
    float getFloat(const std::string key) const;
    glm::vec2 getFloat2(const std::string key) const;
    glm::vec3 getFloat3(const std::string key) const;
    glm::vec4 getFloat4(const std::string key) const;
    glm::ivec2 getInt2(const std::string key) const;

protected:
    // TODO: Use a vector so settings are ordered
    std::map<std::string, Setting> m_settings;

    void validateUniqueSetting(const std::string &name) const;
    void validateKeyExists(const std::string &name) const;
};
