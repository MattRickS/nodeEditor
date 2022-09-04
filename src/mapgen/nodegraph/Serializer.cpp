#include <iomanip>
#include <iostream>

#include "../constants.h"
#include "../log.h"
#include "Serializer.h"

// =============================================================================
// Serializer

StreamSerializer::StreamSerializer(std::ostream *stream) : m_stream(stream) {}

bool StreamSerializer::isOk() const
{
    return m_stream->good();
}
bool StreamSerializer::startObject(const std::string &type)
{
    *m_stream << std::endl;
    indent();
    *m_stream << type << " { ";
    m_currObjectHasProperties = false;
    ++m_nestedDepth;
    return isOk();
}
bool StreamSerializer::finishObject()
{
    if (m_nestedDepth == 0)
    {
        LOG_ERROR("Attempting to finish object without having started one");
        return false;
    }
    --m_nestedDepth;

    // If data was written in the object, last bracket goes on a newline
    if (m_currObjectHasProperties)
    {
        *m_stream << std::endl;
        indent();
    }
    *m_stream << "} ";
    // Whatever object we're inside now has a nested object
    m_currObjectHasProperties = true;
    return isOk();
}

void StreamSerializer::indent()
{
    for (int i = 0; i < m_nestedDepth; ++i)
    {
        *m_stream << "  ";
    }
}
void StreamSerializer::startNewProperty()
{
    if (m_nestedDepth > 0)
    {
        *m_stream << std::endl;
        indent();
    }
    m_currObjectHasProperties = true;
}

// -----------------------------------------------------------------------------
// Types

bool StreamSerializer::writeUInt(unsigned int value)
{
    *m_stream << value << ' ';
    return isOk();
}
bool StreamSerializer::writeBool(bool value)
{
    *m_stream << value << ' ';
    return isOk();
}
bool StreamSerializer::writeInt(int value)
{
    *m_stream << value << ' ';
    return isOk();
}
bool StreamSerializer::writeFloat(float value)
{
    *m_stream << value << ' ';
    return isOk();
}
bool StreamSerializer::writeFloat2(const glm::vec2 &value)
{
    *m_stream << value.x << ' ' << value.y << ' ';
    return isOk();
}
bool StreamSerializer::writeFloat3(const glm::vec3 &value)
{
    *m_stream << value.x << ' ' << value.y << ' ' << value.z << ' ';
    return isOk();
}
bool StreamSerializer::writeFloat4(const glm::vec4 &value)
{
    *m_stream << value.x << ' ' << value.y << ' ' << value.z << ' ' << value.w << ' ';
    return isOk();
}
bool StreamSerializer::writeInt2(const glm::ivec2 &value)
{
    *m_stream << value.x << ' ' << value.y << ' ';
    return isOk();
}
bool StreamSerializer::writeString(const std::string &value)
{
    *m_stream << std::quoted(value) << ' ';
    return isOk();
}

// -----------------------------------------------------------------------------
// Properties

bool StreamSerializer::writePropertyUInt(const std::string &name, unsigned int value)
{
    startNewProperty();
    *m_stream << name << ' ';
    return writeUInt(value);
}
bool StreamSerializer::writePropertyBool(const std::string &name, bool value)
{
    startNewProperty();
    *m_stream << name << ' ';
    return writeBool(value);
}
bool StreamSerializer::writePropertyInt(const std::string &name, int value)
{
    startNewProperty();
    *m_stream << name << ' ';
    return writeInt(value);
}
bool StreamSerializer::writePropertyFloat(const std::string &name, float value)
{
    startNewProperty();
    *m_stream << name << ' ';
    return writeFloat(value);
}
bool StreamSerializer::writePropertyFloat2(const std::string &name, const glm::vec2 &value)
{
    startNewProperty();
    *m_stream << name << ' ';
    return writeFloat2(value);
}
bool StreamSerializer::writePropertyFloat3(const std::string &name, const glm::vec3 &value)
{
    startNewProperty();
    *m_stream << name << ' ';
    return writeFloat3(value);
}
bool StreamSerializer::writePropertyFloat4(const std::string &name, const glm::vec4 &value)
{
    startNewProperty();
    *m_stream << name << ' ';
    return writeFloat4(value);
}
bool StreamSerializer::writePropertyInt2(const std::string &name, const glm::ivec2 &value)
{
    startNewProperty();
    *m_stream << name << ' ';
    return writeInt2(value);
}
bool StreamSerializer::writePropertyString(const std::string &name, const std::string &value)
{
    startNewProperty();
    *m_stream << name << ' ';
    return writeString(value);
}

// =============================================================================
// Deserializer

Deserializer::Deserializer(int version) : m_version(version) {}
int Deserializer::version() const { return m_version; }
void Deserializer::setVersion(int version) { m_version = version; }

StreamDeserializer::StreamDeserializer(std::stringstream *stream) : Deserializer(), m_stream(stream) {}

bool StreamDeserializer::isOk() const
{
    return m_stream->good();
}

bool StreamDeserializer::readUInt(unsigned int &value)
{
    int i;
    bool ok = readInt(i);
    if (ok)
    {
        value = static_cast<unsigned int>(i);
    }
    return ok;
}
bool StreamDeserializer::readBool(bool &value)
{
    int i;
    bool ok = readInt(i);
    if (ok)
    {
        value = static_cast<bool>(i);
    }
    return ok;
}
bool StreamDeserializer::readInt(int &value)
{
    int f;
    bool ok = static_cast<bool>(*m_stream >> f);
    if (ok)
    {
        value = f;
    }
    return ok;
}
bool StreamDeserializer::readFloat(float &value)
{
    float f;
    bool ok = static_cast<bool>(*m_stream >> f);
    if (ok)
    {
        value = f;
    }
    return ok;
}
bool StreamDeserializer::readFloat2(glm::vec2 &value)
{
    float x, y;
    bool ok = readFloat(x) && readFloat(y);
    if (ok)
    {
        value = {x, y};
    }
    return ok;
}
bool StreamDeserializer::readFloat3(glm::vec3 &value)
{
    float x, y, z;
    bool ok = readFloat(x) && readFloat(y) && readFloat(z);
    if (ok)
    {
        value = {x, y, z};
    }
    return ok;
}
bool StreamDeserializer::readFloat4(glm::vec4 &value)
{
    float x, y, z, w;
    bool ok = readFloat(x) && readFloat(y) && readFloat(z) && readFloat(w);
    if (ok)
    {
        value = {x, y, z, w};
    }
    return ok;
}
bool StreamDeserializer::readInt2(glm::ivec2 &value)
{
    int x, y;
    bool ok = readInt(x) && readInt(y);
    if (ok)
    {
        value = {x, y};
    }
    return ok;
}
bool StreamDeserializer::readString(std::string &value)
{
    std::string s;
    bool ok = static_cast<bool>(*m_stream >> std::quoted(s));
    if (ok)
    {
        value = s;
    }
    return ok;
}

bool StreamDeserializer::readProperty(std::string &name)
{
    // Check that the next non-whitespace char isn't the end of the object
    *m_stream >> std::ws;
    char c = m_stream->peek();
    if (m_stream->bad() || c == '}')
    {
        return false;
    }

    std::string text;
    bool ok = static_cast<bool>(*m_stream >> text);
    if (!ok)
    {
        return false;
    }
    name = text;
    return true;
}
bool StreamDeserializer::startReadObject()
{
    *m_stream >> std::ws;
    return m_stream->get() == '{';
}
bool StreamDeserializer::finishReadObject()
{
    *m_stream >> std::ws;
    std::string s;
    bool ok = static_cast<bool>(std::getline(*m_stream, s, '}'));
    if (s.length() > 0)
    {
        LOG_WARNING("Finished reading object but discarded %lu characters", s.length());
    }
    return ok;
}
