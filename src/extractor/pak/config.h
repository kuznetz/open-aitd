#pragma once

#if __cplusplus
#include <vector>
#include <string>
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

#define HAS_YM3812 1

#if defined(HAS_STDINT) || AITD_UE4
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
#else
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;

typedef signed char s8;
typedef signed short int s16;
typedef signed long int s32;
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#ifdef WIN32
#include <search.h>
#endif

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

