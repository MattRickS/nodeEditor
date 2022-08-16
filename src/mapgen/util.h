#pragma once
#include <cstdio>

#include <GL/glew.h>

#include "constants.h"

#define DEBUG_LOG(...) std::fprintf(::stderr, __VA_ARGS__);

void makeQuad(GLuint *VAO);
const char *getChannelName(Channel channel);
