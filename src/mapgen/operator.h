#pragma once
// TODO: Needs to register using the Op defined type
#define REGISTER_OPERATOR(op_name, create_func) \
    bool op_name##_registered = OperatorRegistry::registerOperator(#op_name, (create_func))

#include <functional>
#include <string>
#include <map>
#include <vector>

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "renders.h"
#include "settings.h"
#include "shader.h"

namespace Op
{
    struct Input
    {
        std::string name = "";
        bool required = true;
    };

    struct Output
    {
        std::string layer = "";
    };

    class Operator
    {
    public:
        virtual ~Operator() = default;

        /* Display name for the operator */
        virtual std::string name() const = 0;
        /* Returns the inputs for the operator, which may be optional. Default has no inputs. */
        virtual std::vector<Input> inputs() const;
        /* Returns the layers the operator outputs. Defaults to a single output for the default layer. */
        virtual std::vector<Output> outputs() const;
        /* Returns the texture size the output layers should be */
        virtual glm::ivec2 outputImageSize(const std::vector<Texture *> &inputs, const Settings *const sceneSettings, const Settings *const opSettings);
        /* Populates the default settings for the operator (if any). Defaults to no settings. */
        virtual void defaultSettings(Settings *const settings) const;
        /*
        Called once before any calls to process are made.

        This is not called again unless reset() is called. Default behaviour does nothing.

        @param inputs Vector of pointers to a texture per Input as defined by `inputs()`.
            Optional inputs may receive a nullptr.
        @param outputs Vector of pointers to a texture per Output as defined by `outputs()`.
        @param settings Pointer to the settings to use when processing. Will contain any settings
            defined in `defaultSettings`.
        */
        virtual void preprocess(const std::vector<Texture *> &inputs, const std::vector<Texture *> &outputs, const Settings *const settings);
        /*
        Called by the framework to populate the outputs.

        Returns false if process() should be called again, eg, iterations or incomplete processing.

        @param inputs Vector of pointers to a texture per Input as defined by `inputs()`.
            Optional inputs may receive a nullptr.
        @param outputs Vector of pointers to a texture per Output as defined by `outputs()`.
        @param settings Pointer to the settings to use when processing. Will contain any settings
            defined in `defaultSettings`.
        */
        virtual bool process(const std::vector<Texture *> &inputs,
                             const std::vector<Texture *> &outputs,
                             const Settings *const settings) = 0;
        /*
        Operator should implement to reset any internal state.

        Default behaviour clears any error message, any custom implementation should
        make sure to call the base method.
        After a reset, preprocess will always be called before calls to process.
        */
        virtual void reset();

        const std::string &error();
        void setError(std::string errorMsg);
        bool hasError() const;

    protected:
        std::string m_error;
    };

    /*
    Base compute shader operator which binds all input textures sequentially from 0
    followed by a single output, and sets all settings using the same name.
    */
    class BaseComputeShaderOp : public Operator
    {
    public:
        Shader shader;

        BaseComputeShaderOp(const char *computeShader);
        virtual bool process(const std::vector<Texture *> &inputs,
                             const std::vector<Texture *> &outputs,
                             const Settings *const settings);
    };

    class ContentCreatorComputeShaderOp : public BaseComputeShaderOp
    {
    public:
        ContentCreatorComputeShaderOp(const char *computeShader);

        virtual void defaultSettings(Settings *const settings) const;
        virtual glm::ivec2 outputImageSize([[maybe_unused]] const std::vector<Texture *> &inputs, const Settings *const sceneSettings, const Settings *const opSettings);
    };

    class OperatorRegistry
    {
    public:
        typedef std::function<Operator *()> FactoryFunction;
        typedef std::map<std::string, FactoryFunction> FactoryMap;

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

        class key_iterator : public FactoryMap::iterator
        {
        public:
            key_iterator() : FactoryMap::iterator() {}
            key_iterator(FactoryMap::iterator it) : FactoryMap::iterator(it) {}
            const std::string *operator->() { return &(FactoryMap::iterator::operator->()->first); }
            const std::string &operator*() { return FactoryMap::iterator::operator*().first; }
        };

        static key_iterator begin()
        {
            FactoryMap *map = getFactoryMap();
            return map->begin();
        }

        static key_iterator end()
        {
            FactoryMap *map = getFactoryMap();
            return map->end();
        }

    private:
        static FactoryMap *getFactoryMap()
        {
            static FactoryMap map;
            return &map;
        }
    };
}
