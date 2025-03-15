#pragma once

#if defined(HAS_STDINT)
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

struct ZVStruct
{
	s32 ZVX1;
	s32 ZVX2;
	s32 ZVY1;
	s32 ZVY2;
	s32 ZVZ1;
	s32 ZVZ2;
};
