/*
    dload.c
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "dload.h"
#include "device_connection.h"
#include "dload_constants.h"
#include "endian.h"

int dload_do_echo(uint8_t *buffer, size_t length)
{
    uint8_t *recv_buf = NULL;
    size_t out_size = 0;

    int r = 0;

    uint8_t *msg_buffer = (uint8_t *)malloc(0x8 + length);
    if (msg_buffer == NULL) {
        r = kDload_OutOfMemory;
        goto end;
    }
    dload_hdr_t *hdr = (dload_hdr_t *)msg_buffer;

    hdr->msg_type = LE(BB0_ECHO);
    hdr->msg_length = LE(length);
    memcpy(msg_buffer + 0x8, buffer, length);

    r = dload_device_send_packet(msg_buffer, 0x8 + length);
    if (r < 0) goto end;

    r = dload_device_recv_packet(&recv_buf, &out_size);
    if (r < 0) goto end;

    dload_hdr_t *hdr_recv = (dload_hdr_t *)recv_buf;

    r = kDload_Success;
    if (LE(hdr_recv->msg_type) != BB0_ECHO ||
        out_size != 0x8 + length)
        r = kDload_InvalidResponse;

end:
    if (msg_buffer != NULL)
        free(msg_buffer);
    if (recv_buf != NULL)
        free(recv_buf);
    return r;
}

int dload_get_version(dload_version_t *out_version)
{
    uint8_t *recv_buf = NULL;
    size_t out_size = 0;

    int r = 0;

    uint8_t *msg_buffer = (uint8_t *)malloc(sizeof(dload_hdr_t));
    if (msg_buffer == NULL) {
        r = kDload_OutOfMemory;
        goto end;
    }
    dload_hdr_t *hdr = (dload_hdr_t *)msg_buffer;

    hdr->msg_type = LE(BB0_GET_VERSION);
    hdr->msg_length = LE(0);

    r = dload_device_send_packet(msg_buffer, 0x8);
    if (r < 0) goto end;

    r = dload_device_recv_packet(&recv_buf, &out_size);
    if (r < 0) goto end;

    dload_hdr_t *response = (dload_hdr_t *)recv_buf;
    r = kDload_Success;
    if (LE(response->msg_type) != BB0_REPLY_VERSION ||
        out_size != sizeof(dload_hdr_t) + sizeof(dload_version_t) ||
        LE(response->msg_length) != sizeof(dload_version_t))
        r = kDload_InvalidResponse;
    
    memcpy(out_version, recv_buf + sizeof(dload_hdr_t), sizeof(dload_version_t));

end:
    if (msg_buffer != NULL)
        free(msg_buffer);
    if (recv_buf != NULL)
        free(recv_buf);
    return r;
}

int dload_reset()
{
    int r = 0;

    uint8_t *msg_buffer = (uint8_t *)malloc(0x8);
    if (msg_buffer == NULL) {
        r = kDload_OutOfMemory;
        goto end;
    }
    dload_hdr_t *hdr = (dload_hdr_t *)msg_buffer;

    hdr->msg_type = LE(BB0_RESET);
    hdr->msg_length = LE(0);

    r = dload_device_send_packet(msg_buffer, 0x8);
    if (r < 0) goto end;

    r = kDload_Success;

end:
    if (msg_buffer != NULL)
        free(msg_buffer);
    return r;
}

int dload_power_off()
{
    int r = 0;

    uint8_t *msg_buffer = (uint8_t *)malloc(0x8);
    if (msg_buffer == NULL) {
        r = kDload_OutOfMemory;
        goto end;
    }
    dload_hdr_t *hdr = (dload_hdr_t *)msg_buffer;

    hdr->msg_type = LE(BB0_POWER_OFF);
    hdr->msg_length = LE(0);

    r = dload_device_send_packet(msg_buffer, 0x8);
    if (r < 0) goto end;

    // we don't need to check for a response as the device will power off
    r = kDload_Success;

end:
    if (msg_buffer != NULL)
        free(msg_buffer);
    return r;
}
