#pragma once

#include <glm/glm.hpp>

#include "Texture.h"
#include "../nodegraph/Operator.h"

namespace Op
{
    /*
    Base class for processing OpenGL textures.

    Provides a utility method ensureOutput() and an internal RenderSet which manages
    ownership of output Textures.
    The process method assembles an output RenderSet from the first input's renders with
    the output textures added in (or replacing existing layers). This is available via
    the renderSet() method which can be called on input Operators to access upstream
    layers.
    */
    class RenderSetOperator : public Operator
    {
    public:
        virtual ~RenderSetOperator();

        /* The RenderSet this Operator generates. May include pointers to upstream textures if the layer was not modified. */
        RenderSet_c const *renderSet() const;
        /* Retrieves the Texture pointer from the output RenderSet, or nullptr if layer does not exist. */
        Texture const *layer(const std::string &layer) const;

        virtual void reset();
        /* Attempts to retrieve the image size of the default layer from the first input, falling back on sceneSettings image size. */
        glm::ivec2 outputLayerSize(int outputIndex, const std::vector<RenderSetOperator const *> &inputs, Settings const *sceneSettings);
        /*
        Creates a texture owned by this object for the requested layer or resizes it if it already exists.
        Updates this operator's RenderSet to ensure the layer is added/overridden.
        Method is idempotent.

        If implementing custom layer names, eg, via settings, then reset() should be implemented to
        ensure old layers are deleted to avoid memory leaks.
        */
        Texture *ensureOutputLayer(const std::string &layer, const glm::ivec2 &imageSize);
        /*
        Constructs a vector of the input RenderSets and calls the overloaded process method.
        Derived classes should only implement the overloaded process method.
        Also copies the first input's RenderSet as it's current RenderSet if not yet processed.
        */
        virtual bool process(const std::vector<Operator const *> &inputs, Settings const *settings, Settings const *sceneSettings);
        /*
        Overloaded process method which has already converted inputs to RenderSetOperator for derived classes.
        Guarantees all required inputs are populated and that all connected inputs are RenderSetOperators.
        */
        virtual bool process(const std::vector<RenderSetOperator const *> &inputs, Settings const *settings, Settings const *sceneSettings) = 0;

    protected:
        bool m_renderSetConfigured = false;
        RenderSet m_outputs;
        RenderSet_c m_renderSet;
    };
}
