#pragma once
#include <cstdio>

#include <GL/glew.h>

#include "constants.h"

#define LOG_DEBUG(...)                    \
    std::fprintf(::stderr, "DEBUG   : "); \
    std::fprintf(::stderr, __VA_ARGS__);  \
    std::fprintf(::stderr, "\n");

#define LOG_INFO(...)                     \
    std::fprintf(::stderr, "INFO    : "); \
    std::fprintf(::stderr, __VA_ARGS__);  \
    std::fprintf(::stderr, "\n");

#define LOG_WARNING(...)                  \
    std::fprintf(::stderr, "WARNING : "); \
    std::fprintf(::stderr, __VA_ARGS__);  \
    std::fprintf(::stderr, "\n");

#define LOG_ERROR(...)                    \
    std::fprintf(::stderr, "ERROR   : "); \
    std::fprintf(::stderr, __VA_ARGS__);  \
    std::fprintf(::stderr, "\n");

void makeQuad(GLuint *VAO);
const char *getChannelName(Channel channel);
