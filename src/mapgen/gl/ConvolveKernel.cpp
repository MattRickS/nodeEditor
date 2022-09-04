#include <GL/glew.h>

#include "ConvolveKernel.h"

namespace Op
{
    ConvolveKernel::ConvolveKernel() : m_width(0), m_height(0), m_distribution(nullptr) {}
    ConvolveKernel::ConvolveKernel(int w, int h) : m_width(w), m_height(h), m_distribution(new float[w * h]) {}
    ConvolveKernel::~ConvolveKernel()
    {
        deleteData();
    }

    float &ConvolveKernel::operator[](int index)
    {
        return m_distribution[index];
    }

    int ConvolveKernel::width() const { return m_width; }
    int ConvolveKernel::height() const { return m_height; }
    float *ConvolveKernel::data() const { return m_distribution; }

    void ConvolveKernel::resize(int width, int height)
    {
        deleteData();
        m_width = width;
        m_height = height;
        m_distribution = new float[m_width * m_height];
    }
    void ConvolveKernel::normalise()
    {
        float sum = 0;
        for (int i = 0; i < m_width * m_height; ++i)
        {
            sum += m_distribution[i];
        }
        for (int i = 0; i < m_width * m_height; ++i)
        {
            m_distribution[i] /= sum;
        }
    }

    void ConvolveKernel::readFromTexture(Texture const &texture, Channel channel)
    {
        deleteData();
        m_width = texture.width();
        m_height = texture.height();
        m_distribution = texture.read(channel);
    }
    void ConvolveKernel::loadBuffer(GLenum bufferType, GLenum usage)
    {
        glBufferData(bufferType, sizeof(int) * 2 + sizeof(float) * (m_width * m_height), nullptr, usage);
        int offset = 0;
        glBufferSubData(bufferType, offset, sizeof(int), &m_width);
        offset += sizeof(int);
        glBufferSubData(bufferType, offset, sizeof(int), &m_height);
        offset += sizeof(int);
        glBufferSubData(bufferType, offset, sizeof(float) * m_width * m_height, m_distribution);
    }

    void ConvolveKernel::deleteData()
    {
        if (m_distribution)
        {
            delete[] m_distribution;
        }
    }
}
