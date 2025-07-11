#ifndef UNICORN_DEBUG_H_
#define UNICORN_DEBUG_H_

#include <stdio.h>
#include <assert.h>

#define _DEBUG

#ifdef _DEBUG
#define DBG(x, ...) fprintf(stderr, "%s: " x, __func__, ##__VA_ARGS__)
#else
#define DBG(x, ...)
#endif

#define ERR(x, ...) DBG(x, ##__VA_ARGS__)
#define ASSERT(x) assert(x)
#define DBG_MED(x, ...) DBG(x, ##__VA_ARGS__)
#define DBG_LOW(x, ...) DBG(x, ##__VA_ARGS__)
#define DBG_HIGH(x, ...) DBG(x, ##__VA_ARGS__)

#endif
