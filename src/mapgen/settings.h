#pragma once
#include <map>
#include <string>
#include <variant>

#include <glm/glm.hpp>

typedef std::variant<bool, unsigned int, int, float, glm::vec2, glm::vec3, glm::vec4, glm::ivec2> SettingValue;

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

class Setting
{
public:
    Setting();
    Setting(const std::string name, const SettingType type, SettingValue value);
    const std::string &name() const;
    SettingType type() const;

    template <typename T>
    T value() const;

    template <typename T>
    void setValue(T value);

    void set(SettingValue value);

private:
    std::string m_name;
    SettingType m_type;
    SettingValue m_value;
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
    void registerBool(const std::string name, bool value);
    void registerUInt(const std::string name, unsigned int value);
    void registerInt(const std::string name, int value);
    void registerFloat(const std::string name, float value);
    void registerFloat2(const std::string name, glm::vec2 value);
    void registerFloat3(const std::string name, glm::vec3 value);
    void registerFloat4(const std::string name, glm::vec4 value);
    void registerInt2(const std::string name, glm::ivec2 value);

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
