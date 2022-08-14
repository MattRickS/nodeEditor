#pragma once
#include <string>

const std::string DEFAULT_LAYER = "RGBA";

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
