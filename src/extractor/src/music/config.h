#pragma once

#if __cplusplus
#include <vector>
#include <string>
#define HAS_STDINT
#endif

#ifdef __APPLE__
#include <TargetConditionals.h>
#include <stdint.h>
#define HAS_STDINT
#endif

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
//#include "config.h"
#endif

#ifdef MACOSX
#define UNIX
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include <assert.h>

#ifdef _DEBUG
#define ASSERT(exp) assert(exp)
#else
#define ASSERT(exp)
#endif

#ifdef _DEBUG
#define ASSERT_PTR(exp) assert(exp)
#else
#define ASSERT_PTR(exp)
#endif

