#pragma once
#include <cstdio>

#include <GL/glew.h>

#include "constants.h"

#define LOG_LEVEL 0

#if LOG_LEVEL <= 0
#define LOG_DEBUG(...)                    \
    std::fprintf(::stderr, "DEBUG   : "); \
    std::fprintf(::stderr, __VA_ARGS__);  \
    std::fprintf(::stderr, "\n");
#else
#define LOG_DEBUG(...)
#endif

#if LOG_LEVEL <= 1
#define LOG_INFO(...)                     \
    std::fprintf(::stderr, "INFO    : "); \
    std::fprintf(::stderr, __VA_ARGS__);  \
    std::fprintf(::stderr, "\n");
#else
#define LOG_INFO(...)
#endif

#if LOG_LEVEL <= 2
#define LOG_WARNING(...)                  \
    std::fprintf(::stderr, "WARNING : "); \
    std::fprintf(::stderr, __VA_ARGS__);  \
    std::fprintf(::stderr, "\n");
#else
#define LOG_WARNING(...)
#endif

#if LOG_LEVEL <= 3
#define LOG_ERROR(...)                    \
    std::fprintf(::stderr, "ERROR   : "); \
    std::fprintf(::stderr, __VA_ARGS__);  \
    std::fprintf(::stderr, "\n");
#else
#define LOG_ERROR(...)
#endif

void makeQuad(GLuint *VAO);
const char *getChannelName(Channel channel);
bool containsTextCaseInsensitive(const char *text, const char *search);
