#pragma once
#include <fstream>
#include <string>
#include <sstream>

#include <glm/glm.hpp>

#include "constants.h"

class Serializer
{
public:
    virtual bool isOk() const = 0;
    virtual bool startObject(const std::string &type) = 0;
    virtual bool finishObject() = 0;

    virtual bool writeUInt(unsigned int value) = 0;
    virtual bool writeBool(bool value) = 0;
    virtual bool writeInt(int value) = 0;
    virtual bool writeFloat(float value) = 0;
    virtual bool writeFloat2(const glm::vec2 &value) = 0;
    virtual bool writeFloat3(const glm::vec3 &value) = 0;
    virtual bool writeFloat4(const glm::vec4 &value) = 0;
    virtual bool writeInt2(const glm::ivec2 &value) = 0;
    virtual bool writeString(const std::string &value) = 0;

    virtual bool writePropertyUInt(const std::string &name, unsigned int value) = 0;
    virtual bool writePropertyBool(const std::string &name, bool value) = 0;
    virtual bool writePropertyInt(const std::string &name, int value) = 0;
    virtual bool writePropertyFloat(const std::string &name, float value) = 0;
    virtual bool writePropertyFloat2(const std::string &name, const glm::vec2 &value) = 0;
    virtual bool writePropertyFloat3(const std::string &name, const glm::vec3 &value) = 0;
    virtual bool writePropertyFloat4(const std::string &name, const glm::vec4 &value) = 0;
    virtual bool writePropertyInt2(const std::string &name, const glm::ivec2 &value) = 0;
    virtual bool writePropertyString(const std::string &name, const std::string &value) = 0;
};

class Deserializer
{
public:
    Deserializer(int version = VERSION);

    int version() const;
    void setVersion(int version);
    virtual bool isOk() const = 0;

    virtual bool readUInt(unsigned int &value) = 0;
    virtual bool readBool(bool &value) = 0;
    virtual bool readInt(int &value) = 0;
    virtual bool readFloat(float &value) = 0;
    virtual bool readFloat2(glm::vec2 &value) = 0;
    virtual bool readFloat3(glm::vec3 &value) = 0;
    virtual bool readFloat4(glm::vec4 &value) = 0;
    virtual bool readInt2(glm::ivec2 &value) = 0;
    virtual bool readString(std::string &value) = 0;

    virtual bool readProperty(std::string &name) = 0;
    virtual bool startReadObject() = 0;
    virtual bool finishReadObject() = 0;

private:
    int m_version;
};

class StreamSerializer : public Serializer
{
public:
    StreamSerializer(std::ostream *stream);

    bool isOk() const override;
    bool startObject(const std::string &type) override;
    bool finishObject() override;

    bool writeUInt(unsigned int value) override;
    bool writeBool(bool value) override;
    bool writeInt(int value) override;
    bool writeFloat(float value) override;
    bool writeFloat2(const glm::vec2 &value) override;
    bool writeFloat3(const glm::vec3 &value) override;
    bool writeFloat4(const glm::vec4 &value) override;
    bool writeInt2(const glm::ivec2 &value) override;
    bool writeString(const std::string &value) override;

    bool writePropertyUInt(const std::string &name, unsigned int value) override;
    bool writePropertyBool(const std::string &name, bool value) override;
    bool writePropertyInt(const std::string &name, int value) override;
    bool writePropertyFloat(const std::string &name, float value) override;
    bool writePropertyFloat2(const std::string &name, const glm::vec2 &value) override;
    bool writePropertyFloat3(const std::string &name, const glm::vec3 &value) override;
    bool writePropertyFloat4(const std::string &name, const glm::vec4 &value) override;
    bool writePropertyInt2(const std::string &name, const glm::ivec2 &value) override;
    bool writePropertyString(const std::string &name, const std::string &value) override;

protected:
    std::ostream *m_stream;
    int m_nestedDepth = 0;
    bool m_currObjectHasProperties = true;

    void indent();
    void startNewProperty();
};

class StreamDeserializer : public Deserializer
{
public:
    StreamDeserializer(std::stringstream *stream);

    bool isOk() const override;

    bool readUInt(unsigned int &value) override;
    bool readBool(bool &value) override;
    bool readInt(int &value) override;
    bool readFloat(float &value) override;
    bool readFloat2(glm::vec2 &value) override;
    bool readFloat3(glm::vec3 &value) override;
    bool readFloat4(glm::vec4 &value) override;
    bool readInt2(glm::ivec2 &value) override;
    bool readString(std::string &value) override;

    bool readProperty(std::string &name) override;
    bool startReadObject() override;
    bool finishReadObject() override;

protected:
    std::stringstream *m_stream;
};
