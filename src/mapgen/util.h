#pragma once

#include "constants.h"

const char *getChannelName(Channel channel);
bool textMatchesCaseInsensitive(const char *text, const char *search);
bool containsTextCaseInsensitive(const char *text, const char *search);
