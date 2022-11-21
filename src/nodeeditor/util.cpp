#include <cstring>

#include "constants.h"
#include "util.h"

const char *getChannelName(Channel channel)
{
    switch (channel)
    {
    case Channel_All:
        return "RGBA";
    case Channel_Red:
        return "Red";
    case Channel_Green:
        return "Green";
    case Channel_Blue:
        return "Blue";
    case Channel_Alpha:
        return "Alpha";
    default:
        return "";
    }
}

bool textMatchesCaseInsensitive(const char *text, const char *search)
{
    size_t i1 = 0;
    for (; text[i1] != '\0' && search[i1] != '\0'; ++i1)
    {
        if (std::tolower(static_cast<unsigned char>(text[i1])) != std::tolower(static_cast<unsigned char>(search[i1])))
        {
            return false;
        }
    }

    return text[i1] == '\0' && search[i1] == '\0';
}

bool containsTextCaseInsensitive(const char *text, const char *search)
{
    for (size_t i1 = 0; text[i1] != '\0'; ++i1)
    {
        size_t i2 = 0;
        for (; search[i2] != '\0'; ++i2)
        {
            // Search does not fit in the rest of the text
            if (text[i1 + i2] == '\0')
            {
                return false;
            }
            // Break on the first mismatching character
            if (std::tolower(static_cast<unsigned char>(text[i1 + i2])) != std::tolower(static_cast<unsigned char>(search[i2])))
            {
                break;
            }
            // Keep incrementing search index as long as they match
        }
        // The whole search string was matched successfully
        if (search[i2] == '\0')
        {
            return true;
        }
    }

    return false;
}
