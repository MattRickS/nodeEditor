#pragma once
#include <GL/glew.h>
#include "Texture.h"

namespace Op
{
    class ConvolveKernel
    {
    public:
        ConvolveKernel();
        ConvolveKernel(int w, int h);
        ~ConvolveKernel();

        float &operator[](int index);

        int width() const;
        int height() const;
        float *data() const;

        void resize(int width, int height);
        void normalise();

        void readFromTexture(Texture const &texture, Channel channel);

        // Loads to the currently bound buffer matching bufferType
        void loadBuffer(GLenum bufferType, GLenum usage);

    protected:
        int m_width;
        int m_height;
        float *m_distribution;

        void deleteData();
    };
}
