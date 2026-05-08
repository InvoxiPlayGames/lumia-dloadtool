#ifndef DLOAD_H
#define DLOAD_H

#include <stddef.h>
#include <stdint.h>

typedef enum _dload_errors_t {
    // success status code is always 0
    kDload_Success = 0,
    // response errors
    kDload_InvalidResponse = -1,
    // misc random errors
    kDload_OutOfMemory = -4444,
} dload_errors_t;

typedef struct _dload_hdr_t
{
    uint32_t msg_type;
    uint32_t msg_length;
} dload_hdr_t;

typedef struct _dload_version_t
{
    uint8_t unk[0xC];
    uint32_t timestamp;
} dload_version_t;

int dload_do_echo(uint8_t *buffer, size_t length);
int dload_get_version(dload_version_t *out_version);
int dload_reset();
int dload_power_off();

#endif // DLOAD_H
