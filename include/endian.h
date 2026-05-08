#ifndef ENDIAN_H_
#define ENDIAN_H_

//#define SHOULD_BE_BE

// TODO(Emma): look at getting help from the compiler where we can for this

#ifdef SHOULD_BE_BE
#define LE16(i) ((((i) & 0xFF) << 8 | ((i) >> 8) & 0xFF) & 0xFFFF)
#define LE(i)   (((i) & 0xff) << 24 | ((i) & 0xff00) << 8 | ((i) & 0xff0000) >> 8 | ((i) >> 24) & 0xff)
#define LE64(i) (LE((i) & 0xFFFFFFFF) << 32 | LE(((i) >> 32) & 0xFFFFFFFF))
#else
#define LE16(i) i
#define LE(i) i
#define LE64(i) i
#endif

#ifndef SHOULD_BE_BE
#define BE16(i) ((((i) & 0xFF) << 8 | ((i) >> 8) & 0xFF) & 0xFFFF)
#define BE(i)   (((i) & 0xff) << 24 | ((i) & 0xff00) << 8 | ((i) & 0xff0000) >> 8 | ((i) >> 24) & 0xff)
#define BE64(i) (LE((i) & 0xFFFFFFFF) << 32 | LE(((i) >> 32) & 0xFFFFFFFF))
#else
#define BE16(i) i
#define BE(i) i
#define BE64(i) i
#endif

#endif // ENDIAN_H_
