#pragma once
#include <string>
#include <vector>

#include "../constants.h"
#include "../operator.h"
#include "../nodegraph/Settings.h"
#include "../util.h"

/*
This is a weird one.
- It doesn't want to create any outputs but needs an output port
- It wants to modify the renderset based on input rendersets.

Generic base class Processor defines
    vector<Input> inputs()
    bool process(vector<Processor *>)
    setError/hasError/error

Perhaps a RenderSetProcessor should be:
    registerSettings(Settings*)
    ensureOutput(layer, size)
    bool process(vector<renderSet*>, settings, sceneSettings)
        manage own output Textures
        <populates local renderSet>

And an ImageProcessor subclass for these
    imageSize()
    vector<Output> outputLayers(settings, sceneSettings)  // where Output(layer, fixedSize=null)
    bool renderOutputs(vector<Texture*>, vector<Output*>, settings, sceneSettings)
*/
namespace Op
{
    class Copy : public Operator
    {
    public:
        static Copy *create()
        {
            return new Copy();
        }

        std::string name() const override { return "Copy"; }
        std::vector<Input> inputs() const override
        {
            return {{}};
        }
        void registerSettings(Settings *const settings) const override
        {
            settings->registerString("filepath", "");
        }

        bool process(const std::vector<Texture *> &inputs,
                     [[maybe_unused]] const std::vector<Texture *> &outputs,
                     Settings const *settings) override
        {
            return true;
        }
    };

    REGISTER_OPERATOR(Copy, Copy::create);
}
