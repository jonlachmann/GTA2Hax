#pragma once

typedef unsigned char u8;
typedef signed char s8;
typedef char char_type;

typedef unsigned short int u16;
typedef signed short int s16;

typedef unsigned int u32;
typedef signed int s32;

typedef float f32;
typedef double f64;


#if defined(GTA2_PORT) && (defined(__LP64__) || defined(_WIN64))
// gta2_re's 64-bit port: the structures that hold pointers grow, the sizes are the 32-bit ones
#define STATIC_ASSERT(x, msg)
#else
#define STATIC_ASSERT(x, msg) int static_assert_(int static_assert_[(x)?1:-1])
#endif