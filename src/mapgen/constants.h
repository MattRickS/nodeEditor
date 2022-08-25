#pragma once
#include <string>
#include <unordered_map>

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

enum SelectFlag
{
    SelectFlag_None = 0,
    SelectFlag_Hover = 1 << 0,
    SelectFlag_Select = 1 << 1,
    SelectFlag_View = 1 << 2,
};

enum class State
{
    Unprocessed,
    Preprocessing,
    Processing,
    Processed,
    Error,
};

enum FileType
{
    FileType_None,
    FileType_PNG,
    FileType_HDR
};

const std::string EXTENSION_HDR = ".hdr";
const std::string EXTENSION_PNG = ".png";
const std::unordered_map<FileType, std::string> FILE_TYPES{
    {FileType_HDR, EXTENSION_HDR},
    {FileType_PNG, EXTENSION_PNG},
};
