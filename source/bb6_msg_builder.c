#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "bb6_msg_builder.h"
#include "bb6_tlv.h"
#include "dload_constants.h"
#include "dload.h"
#include "endian.h"

static int bb6_sequence_num = 0;

int bb6_add_tlv(bb6_msg_inprog_t * request, uint32_t type, void *data, size_t length)
{
    if (request == NULL)
        return -1;
    bb6_tlv_inprog_t *target = NULL;
    bool target_is_new = false;

    // check if we already have a TLV of this type
    for (int i = 0; i < request->tlv_num; i++) {
        if (request->tlv[i].type == type) {
            target = &request->tlv[i];
            // free if we have a value
            if (request->tlv[i].value != NULL)
                free(request->tlv[i].value);
            break;
        }
    }
    // instance a new one
    if (target == NULL) {
        // .. unless we have the max already
        if (request->tlv_num >= MAX_TLV)
            return -2;
        target = &request->tlv[request->tlv_num];
        target_is_new = true;
        request->tlv_num++;
    }
    // copy out into a new buffer
    void *newbuf = NULL;
    if (data != NULL && length > 0) {
        newbuf = malloc(length);
        if (newbuf == NULL) {
            if (target_is_new) request->tlv_num--;
            return -3;
        }
        memcpy(newbuf, data, length);
    }
    target->type = type;
    target->length = newbuf != NULL ? length : 0;
    target->value = newbuf;

    return 0;
}

bb6_msg_inprog_t *bb6_alloc_request(uint32_t command)
{
    // allocate and initialise a new request object
    bb6_msg_inprog_t *newmsg = (bb6_msg_inprog_t *)malloc(sizeof(bb6_msg_inprog_t));
    if (newmsg == NULL)
        return NULL;
    memset(newmsg, 0, sizeof(bb6_msg_inprog_t));
    
    // add the request TLV with the command ID and sequence number
    BB6_REQUEST_TLV_t request = {
        .sequence = bb6_sequence_num,
        .command = command,
    };
    int r = bb6_add_tlv(newmsg, BB6_REQUEST_TLV, &request, sizeof(BB6_REQUEST_TLV_t));
    if (r != 0) {
        free(newmsg);
        return NULL;
    }

    bb6_sequence_num++;
    return newmsg;
}

uint8_t *bb6_serialize_request(bb6_msg_inprog_t *request, size_t *out_len)
{
    if (request == NULL)
        return NULL;

    // find out how big we have to make our buffer
    size_t buf_sz = sizeof(dload_hdr_t) + sizeof(BB6_HEADER_t) + (request->tlv_num * 0x8);
    for (int i = 0; i < request->tlv_num; i++)
        buf_sz += request->tlv[i].length;

    // allocate the buffer
    uint8_t *out_buf = (uint8_t *)malloc(buf_sz);
    if (out_buf == NULL)
        return NULL;

    // build out the header
    uint8_t *data_ptr = out_buf;
    // message header
    dload_hdr_t *dld_hdr = (dload_hdr_t *)data_ptr;
    dld_hdr->msg_type = LE(BB6_MAGIC);
    dld_hdr->msg_length = LE(sizeof(BB6_HEADER_t));
    data_ptr += sizeof(dload_hdr_t);
    // BB6 header
    BB6_HEADER_t *bb6_hdr = (BB6_HEADER_t *)data_ptr;
    bb6_hdr->unk_0x1 = LE(0x1);
    bb6_hdr->msg_type = LE(BB6_MSG_TYPE_REQ);
    bb6_hdr->num_tlv = LE(request->tlv_num);
    data_ptr += sizeof(BB6_HEADER_t);
    // the TLVs themselves
    for (int i = 0; i < request->tlv_num; i++) {
        *(uint32_t *)(data_ptr + 0x0) = LE(request->tlv[i].type);
        *(uint32_t *)(data_ptr + 0x4) = LE(request->tlv[i].length);
        data_ptr += 0x8;
        if (request->tlv[i].length > 0 && request->tlv[i].value != NULL) {
            memcpy(data_ptr, request->tlv[i].value, request->tlv[i].length);
            data_ptr += request->tlv[i].length;
        }
    }

    // return the buffer we built
    if (out_len != NULL)
        *out_len = buf_sz;
    return out_buf;
}

void bb6_free_request(bb6_msg_inprog_t *request)
{
    if (request == NULL)
        return;

    // free all the allocated buffers
    for (int i = 0; i < MAX_TLV; i++) {
        if (request->tlv[i].value != NULL)
            free(request->tlv[i].value);
    }
    free(request);

    return;
}
