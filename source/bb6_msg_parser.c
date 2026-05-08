#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "bb6_tlv.h"
#include "dload_constants.h"

// TODO(Emma): give this code a heavy read-over because i'm not the proudest of this
//   security isn't a massive concern given it's a semi-trustworthy USB device but it would be nice to be safe
//   plus i don't know how reliable this is..

const void *bb6_get_packet_tlv(const uint8_t *buf, size_t buf_size, uint32_t find_type, size_t *out_len)
{
    // minimum size check
    if (buf_size < (0x8 + sizeof(BB6_HEADER_t)))
        return NULL;

    // bb6 magic check
    if (*(uint32_t *)(buf + 0x0) != LE(BB6_MAGIC) || 
        *(uint32_t *)(buf + 0x4) != LE(sizeof(BB6_HEADER_t)))
        return NULL;
    // bb6 header check
    BB6_HEADER_t *bb6_hdr = (BB6_HEADER_t *)(buf + 0x8);
    if (LE(bb6_hdr->unk_0x1) != 0x1 ||
        (LE(bb6_hdr->msg_type) != BB6_MSG_TYPE_RES && LE(bb6_hdr->msg_type) != BB6_MSG_TYPE_REQ))
        return NULL;

    const uint8_t *cur_ptr = buf + 0x8 + sizeof(BB6_HEADER_t);
    const uint8_t *end_ptr = buf + buf_size;
    for (int i = 0; i < LE(bb6_hdr->num_tlv); i++) {
        // range check
        if (cur_ptr >= end_ptr || (cur_ptr + 0x8) > end_ptr)
            break;
        uint32_t type = LE(*(uint32_t *)(cur_ptr + 0x0));
        uint32_t length = LE(*(uint32_t *)(cur_ptr + 0x4));
        cur_ptr += 0x8;
        // if we're the last TLV in the list then it's probably safe
        //   to say the length is the remaining size of the packet
        if (length == 0 && i == (bb6_hdr->num_tlv - 1))
            length = end_ptr - cur_ptr; // pointer math? dame da ne...

        if (type == find_type && (cur_ptr + length) <= end_ptr) {
            if (out_len != NULL)
                *out_len = length;

            return cur_ptr;
        }
        cur_ptr += length;
    }

    return NULL;
}

int bb6_get_return_code(const uint8_t *buf, size_t buf_size)
{
    size_t rsize = 0;
    BB6_RESPONSE_TLV_t *response = (BB6_RESPONSE_TLV_t *)bb6_get_packet_tlv(buf, buf_size, BB6_RESPONSE_TLV, &rsize);
    if (response == NULL || rsize != sizeof(BB6_RESPONSE_TLV_t))
        return -1; // no real dload error code has this so it's fine

    return LE(response->rcode);
}
