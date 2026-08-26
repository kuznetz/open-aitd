#ifndef _COMMON_H_
#define _COMMON_H_

#include "config.h"

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;
typedef signed char S8;
typedef signed short S16;
typedef signed long S32;

#ifdef UNIX
#define FORCEINLINE static inline
#else
#ifdef WIN32
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE inline
#endif
#endif

FORCEINLINE u8 READ_LE_U8(void *ptr)
{
    return *(u8*)ptr;
}

FORCEINLINE s8 READ_LE_S8(void *ptr)
{
    return *(s8*)ptr;
}

FORCEINLINE u16 READ_LE_U16(void *ptr)
{
#ifdef MACOSX
  return (((u8*)ptr)[1]<<8)|((u8*)ptr)[0];
#else
  return *(u16*)ptr;
#endif
}

FORCEINLINE s16 READ_LE_S16(void *ptr)
{
  return (s16)READ_LE_U16(ptr);
}

FORCEINLINE u16 READ_BE_U16(void *ptr)
{
#ifdef MACOSX
  return *(u16*)ptr;
#else
  return (((u8*)ptr)[0]<<8)|((u8*)ptr)[1];
#endif
}

FORCEINLINE s16 READ_BE_S16(void *ptr)
{
  return (s16)READ_BE_U16(ptr);
}

FORCEINLINE u32 READ_LE_U32(void *ptr)
{
#ifdef MACOSX
  return (((u8*)ptr)[3]<<24)|(((u8*)ptr)[2]<<16)|(((u8*)ptr)[1]<<8)|((u8*)ptr)[0];
#else
  return *(u32*)ptr;
#endif
}

FORCEINLINE s32 READ_LE_S32(void *ptr)
{
  return (s32)READ_LE_U32(ptr);
}

FORCEINLINE u32 READ_BE_U32(void *ptr)
{
#ifdef MACOSX
  return *(u32*)ptr;
#else
  return (((u8*)ptr)[3]<<24)|(((u8*)ptr)[2]<<16)|(((u8*)ptr)[1]<<8)|((u8*)ptr)[0];
#endif
}

FORCEINLINE s32 READ_BE_S32(void *ptr)
{
  return (s32)READ_LE_U32(ptr);
}

#endif
