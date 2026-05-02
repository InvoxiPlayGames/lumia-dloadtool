#ifndef BB6_TLV_H
#define BB6_TLV_H

#include <stdint.h>

typedef struct _BB6_HEADER_t {
    uint32_t unk_0x1;
    uint32_t unk_0x2;
    uint32_t num_tlv;
} BB6_HEADER_t;

typedef struct _BB6_REQUEST_TLV_t {
    uint32_t command;
    uint32_t sequence;
} BB6_REQUEST_TLV_t;

typedef struct _BB6_RESPONSE_TLV_t {
    uint32_t sequence;
    uint32_t rcode;
    uint32_t unk;
} BB6_RESPONSE_TLV_t;

typedef struct _BB6_CERT_TLV_t {
    uint32_t length;
    uint32_t unk;
    uint32_t key_id;
} BB6_CERT_TLV_t;

typedef struct _BB6_IMAGE_TLV_t {
    uint32_t size_lo; // size of current block of data
    uint32_t size_hi;
    uint32_t addr_lo; // address in flash for the write to occur?
    uint32_t addr_hi;
    uint32_t offset_lo; // offset from address in flash
    uint32_t offset_hi;
    // these two values seem to be key-related
    uint32_t unk1; // 0x4
    uint32_t unk2; // 0x1
} BB6_IMAGE_TLV_t;

typedef struct _BB6_GET_NOKIA_KEY_TLV_t {
    uint32_t key_id;
} BB6_GET_NOKIA_KEY_TLV_t;

typedef struct _BB6_SUBMIT_NOKIA_SIG_TLV_t {
    uint32_t key_id; // potentially
    uint32_t unk2;
    uint8_t unk3[0x8];
    uint8_t unk4[0x18]; // part of data returned in response to BB6_GET_NOKIA_KEY_TLV
    uint8_t signature[0x40];
} BB6_SUBMIT_NOKIA_SIG_TLV;
int a = sizeof(BB6_SUBMIT_NOKIA_SIG_TLV);

#endif // BB6_TLV_H
