#ifndef BB6_MSG_PARSER_H
#define BB6_MSG_PARSER_H

#include <stdint.h>
#include <stddef.h>

const void *bb6_get_packet_tlv(const uint8_t *buf, size_t buf_size, uint32_t find_type, size_t *out_len);
int bb6_get_return_code(const uint8_t *buf, size_t buf_size);

#endif // BB6_MSG_PARSER_H
