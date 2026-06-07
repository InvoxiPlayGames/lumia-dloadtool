/*
    dload.h
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

#ifndef DLOAD_H
#define DLOAD_H

#include <stddef.h>
#include <stdint.h>
#include "rm_cert.h"

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
int dload_reset(void);
int dload_power_off(void);

//bb6 msgs
int dload_do_control(uint32_t ctrl_reg, uint32_t ctrl_val);
int dload_send_cert(rm_cert_t *cert);
int dload_flash_write_delayed(uint64_t address, uint64_t offset, void *data, size_t length);
int dload_flush_delayed(void);
int dload_reset_osbl(void);

#endif // DLOAD_H
