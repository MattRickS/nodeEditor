#pragma once
#include <vector>

#include <GL/glew.h>

#include "renders.h"

class Operator
{
protected:
    std::vector<Texture> outputs;
    unsigned int m_width, m_height;
    GLuint FBO;

public:
    // TODO: Should have a virtual destructor so that it can be stored as the base class

    /*
    Separate init method used so that it can use the virtual in/out layers methods
    which would be undefined in the constructor.
    */
    void init(unsigned int width, unsigned int height);
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
    Convenience method for populating the renderset that assumes texture outputs
    are 1:1 with the outLayers. Must be overridden if different behaviour is required.
    */
    virtual void PopulateRenderSet(RenderSet *renderSet);
};
