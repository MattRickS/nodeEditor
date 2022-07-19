#pragma once
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "renders.h"

typedef std::variant<bool, unsigned int, int, float, glm::vec2, glm::vec3, glm::vec4, glm::ivec2> SettingValue;

class Settings
{
protected:
    std::map<std::string, SettingValue> m_settings;

public:
    template <typename T>
    void Register(std::string key, T defaultValue)
    {
        // Key can only be registered once
        if (m_settings.find(key) != m_settings.end())
        {
            throw std::invalid_argument(key);
        }
        m_settings[key] = defaultValue;
    }
    bool Set(std::string key, SettingValue value)
    {
        if (m_settings.find(key) == m_settings.end())
        {
            return false;
        }
        m_settings[key] = value;
        return true;
    }
    template <typename T>
    T Get(std::string key)
    {
        if (m_settings.find(key) == m_settings.end())
        {
            throw std::out_of_range(key);
        }
        return std::get<T>(m_settings[key]);
    }
};

enum OpType
{
    OP_TERRAIN_GEN,
    OP_INVERT,
};

class Operator
{
protected:
    std::vector<Texture> outputs;
    unsigned int m_width, m_height;
    GLuint FBO;

public:
    Settings settings;
    // TODO: Should have a virtual destructor so that it can be stored as the base class

    /*
    Separate init method used so that it can use the virtual in/out layers methods
    which would be undefined in the constructor.
    */
    void init(unsigned int width, unsigned int height);
    /* Type of the operator */
    virtual OpType type() const = 0;
    /* Display name for the operator */
    virtual std::string name() const = 0;
    /*
    Resizes the operators outputs. Data preservation is optional as the framework
    will try to reprocess the outputs.
    */
    void resize(unsigned int width, unsigned int height);

    /*
    List of the layer types this operator requires.
    */
    virtual std::vector<Layer> inLayers() const = 0;
    /*
    List of the layer types this operator produces.
    */
    virtual std::vector<Layer> outLayers() const = 0;
    /*
    Called by the framework to generate outputs and populate/update the renderset layers
    */
    virtual void process(RenderSet *renders) = 0;
    /*
    process() is called repeatedly until this returns true
    */
    virtual bool isProcessed() const = 0;
    /*
    Convenience method for populating the renderset that assumes texture outputs
    are 1:1 with the outLayers. Must be overridden if different behaviour is required.
    */
    virtual void PopulateRenderSet(RenderSet *renderSet);
};
