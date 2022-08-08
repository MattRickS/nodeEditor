#pragma once
#define REGISTER_OPERATOR(op_name, create_func) \
    bool op_name##_registered = OperatorRegistry::registerOperator(#op_name, (create_func))

#include <functional>
#include <map>
#include <string>
#include <variant>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "renders.h"
#include "settings.h"

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
    virtual void init(unsigned int width, unsigned int height);
    /* Display name for the operator */
    virtual std::string name() const = 0;
    /*
    Resizes the operators outputs. Data preservation is optional as the framework
    will try to reprocess the outputs.
    */
    virtual void resize(unsigned int width, unsigned int height);

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
    virtual bool process(RenderSet *renders) = 0;
    /*
    Called once before calls to process are made. Is not called again unless
    reset() is called.

    Pseudo example of how this is called

        preprocess()
        while (!process());
    */
    virtual void preprocess(RenderSet *renders);
    /*
    Operator should implement to reset any internal state.

    After a reset, preprocess will always be called before calls to process.
    */
    virtual void reset() = 0;
    /*
    Convenience method for populating the renderset that assumes texture outputs
    are 1:1 with the outLayers. Must be overridden if different behaviour is required.
    */
    virtual void PopulateRenderSet(RenderSet *renderSet);
};

class OperatorRegistry
{
public:
    typedef std::function<Operator *()> FactoryFunction;
    typedef std::unordered_map<std::string, FactoryFunction> FactoryMap;

    static bool registerOperator(const std::string &name, FactoryFunction func)
    {
        FactoryMap *map = getFactoryMap();
        if (map->find(name) != map->end())
        {
            return false;
        }
        (*map)[name] = func;
        return true;
    }

    static Operator *create(const std::string &name)
    {
        FactoryMap *map = getFactoryMap();
        if (map->find(name) == map->end())
        {
            return nullptr;
        }
        return (*map)[name]();
    }

private:
    static FactoryMap *getFactoryMap()
    {
        static FactoryMap map;
        return &map;
    }
};
