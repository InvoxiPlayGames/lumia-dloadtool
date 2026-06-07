/*
    bb6_msg_builder.h
    Part of lumia-dloadtool
    Copyright (C) 2026 Emma / InvoxiPlayGames

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, see <https://www.gnu.org/licenses/>.
*/

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
