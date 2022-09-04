#pragma once
#include <string>
#include <unordered_map>

const float PI = 3.14159265358979f;
const unsigned int VERSION = 1;
const unsigned int DEFAULT_WIDTH = 1024;
const unsigned int DEFAULT_HEIGHT = 1024;
const std::string KEY_VERSION = "version";
const std::string KEY_GRAPH = "Graph";
const std::string KEY_NODES = "nodes";
const std::string KEY_SETTINGS = "settings";
const std::string KEY_INPUTS = "inputs";
const std::string KEY_INPUT = "i";
const std::string KEY_NODE_ID = "id";
const std::string KEY_NODE_POS = "pos";
const std::string KEY_NODE_FLAGS = "flags";

const std::string DEFAULT_LAYER = "RGBA";
const std::string SCENE_SETTING_IMAGE_SIZE = "imageSize";

enum Channel
{
    Channel_All = -1,
    Channel_Red = 0,
    Channel_Green = 1,
    Channel_Blue = 2,
    Channel_Alpha = 3,
    Channel_Last
};

enum ChannelMask
{
    ChannelMask_None = 0,
    ChannelMask_Red = 1 << 0,
    ChannelMask_Green = 1 << 1,
    ChannelMask_Blue = 1 << 2,
    ChannelMask_Alpha = 1 << 3,
    ChannelMask_RGB = ChannelMask_Red | ChannelMask_Green | ChannelMask_Blue,
    ChannelMask_RGBA = ChannelMask_RGB | ChannelMask_Alpha,
    ChannelMask_Last
};

enum SelectFlag
{
    SelectFlag_None = 0,
    SelectFlag_Hover = 1 << 0,
    SelectFlag_Select = 1 << 1,
    SelectFlag_View = 1 << 2,
};

enum FileType
{
    FileType_None,
    FileType_PNG,
    FileType_HDR
};

enum SerializeType
{
    SerializeType_Object,
    SerializeType_Bool,
    SerializeType_Float,
    SerializeType_Float2,
    SerializeType_Float3,
    SerializeType_Float4,
    SerializeType_Int,
    SerializeType_Int2,
    SerializeType_UInt,
    SerializeType_String
};

const std::string EXTENSION_HDR = ".hdr";
const std::string EXTENSION_PNG = ".png";
const std::unordered_map<FileType, std::string> FILE_TYPES{
    {FileType_HDR, EXTENSION_HDR},
    {FileType_PNG, EXTENSION_PNG},
};
