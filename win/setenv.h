#pragma once

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline int setenv(const char *name, const char *value, int overwrite) {
    assert(overwrite == 1);
    
    size_t envStringLength = strlen(name) + strlen(value) + 1;
	char envStringBuffer[envStringLength];
	sprintf(envStringBuffer, "%s=%s", name, value);
	return _putenv(envStringBuffer);
}