#ifndef BB6_TLV_H
#define BB6_TLV_H

#include <stdint.h>

typedef struct _BB6_HEADER_t {
    uint32_t unk_0x1;
    uint32_t msg_type; // always 0x2 on send, 0x3 on recv
    uint32_t num_tlv;
} BB6_HEADER_t;

typedef struct _BB6_REQUEST_TLV_t {
    uint32_t sequence;
    uint32_t command;
} BB6_REQUEST_TLV_t;

typedef struct _BB6_RESPONSE_TLV_t {
    uint32_t sequence;
    uint32_t rcode;
    uint32_t unk;
} BB6_RESPONSE_TLV_t;

typedef struct _BB6_CERT_TLV_t {
    uint32_t length;
    uint32_t unk; // 0x4, doesn't get checked
    uint32_t key_id; // 0x1
} BB6_CERT_TLV_t;

typedef struct _BB6_IMAGE_TLV_t {
    uint64_t size; // size of current block of data
    uint64_t addr; // address in flash for the write to occur?
    uint64_t offset; // offset from address in flash
    // these two values seem to be key-related
    uint32_t unk; // 0x4
    uint32_t key_id; // 0x1
} BB6_IMAGE_TLV_t;

typedef struct _BB6_CONTROL_TLV_t {
    uint32_t control_id;
    uint32_t length; // always 4 except 0x10001 but it doesn't check
    uint32_t value;
} BB6_CONTROL_TLV_t;

typedef struct _BB6_GET_NOKIA_KEY_TLV_t {
    uint32_t unk;
    uint32_t key_id;
} BB6_GET_NOKIA_KEY_TLV_t;

typedef struct _BB6_RESPONSE_NOKIA_KEY_TLV_t {
    uint32_t unk1;
    uint32_t unk2;
    uint8_t hash[0x14];
    uint8_t random[0x18];
} BB6_RESPONSE_NOKIA_KEY_TLV_t;

typedef struct _BB6_SUBMIT_NOKIA_SIG_TLV_t {
    uint32_t key_id; // potentially
    uint32_t unk2;
    uint8_t unk3[0x8];
    uint8_t unk4[0x18]; // part of data returned in response to BB6_GET_NOKIA_KEY_TLV
    uint8_t signature[0x40];
} BB6_SUBMIT_NOKIA_SIG_TLV_t;

#endif // BB6_TLV_H
