/*
    bb6_msg_parser.h
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

#ifndef BB6_MSG_PARSER_H
#define BB6_MSG_PARSER_H

#include <stdint.h>
#include <stddef.h>

const void *bb6_get_packet_tlv(const uint8_t *buf, size_t buf_size, uint32_t find_type, size_t *out_len);
int bb6_get_return_code(const uint8_t *buf, size_t buf_size);

#endif // BB6_MSG_PARSER_H
