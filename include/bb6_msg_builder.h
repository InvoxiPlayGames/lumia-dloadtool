#ifndef BB6_MSG_BUILDER_H
#define BB6_MSG_BUILDER_H

#include <stdint.h>
#include <stddef.h>

#define MAX_TLV 6

typedef struct _bb6_tlv_inprog_t {
    uint32_t type;
    size_t length;
    void *value;
} bb6_tlv_inprog_t;

typedef struct _bb6_msg_inprog_t {
    int tlv_num;
    bb6_tlv_inprog_t tlv[MAX_TLV];
} bb6_msg_inprog_t;

int bb6_add_tlv(bb6_msg_inprog_t * request, uint32_t type, void *data, size_t length);
bb6_msg_inprog_t *bb6_alloc_request(uint32_t command);
uint8_t *bb6_serialize_request(bb6_msg_inprog_t *request, size_t *out_len);
void bb6_free_request(bb6_msg_inprog_t *request);

#endif // BB6_MSG_BUILDER_H
