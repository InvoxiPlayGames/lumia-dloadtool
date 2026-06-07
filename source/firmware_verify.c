/*
    firmware_verify.c
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

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "endian.h"
#include "rm_cert.h"

// TODO(Emma): use system SHA256 libraries when available rather than using existing code
#include "sha256.h"

int verify_chunk_checksum(rm_cert_t *cert, int chunk_i, FILE *fp, long offset)
{
    if (cert->chunk_hdr == NULL || cert->chunks == NULL) return -10; // no chunk data
    
    int total_chunk_count = LE(cert->chunk_hdr->count) & 0x7FFFFFFF;
    if (chunk_i >= total_chunk_count) return -2; // invalid chunk

    if (LE(cert->chunk_hdr->hash_type) == kHashSha256) {
        // get the chunk's info
        rm_cert_chunk_sha256_t *chunk = &((rm_cert_chunk_sha256_t *)cert->chunks)[chunk_i];
        long chunk_offset = LE64(chunk->start) + offset;
        long chunk_size = LE64(chunk->end) - LE64(chunk->start) + 1;
        // allocate a buffer for the chunk
        void *chunk_data = malloc(chunk_size);
        if (chunk_data == NULL)
            return -3; // out of memory
        if (fseek(fp, chunk_offset, SEEK_SET) != 0) {
            free(chunk_data);
            return -6; // failed to seek to chunk
        }
        if (fread(chunk_data, 1, chunk_size, fp) != chunk_size) {
            free(chunk_data);
            return -4; // failed to read chunk from file
        }
        // get the sha256 hash of it
        uint8_t s256_out[0x20];
        sha256(chunk_data, chunk_size, s256_out);
        free(chunk_data);
        // compare to original
        if (memcmp(chunk->sha256, s256_out, 0x20) != 0)
            return -5; // invalid checksum
        return 0;
    } else {
        return -1; //unsupported hash type
    }
}

int verify_firmware_checksum(rm_cert_t *cert, FILE *fp, long offset)
{
    if (cert->chunk_hdr == NULL || cert->chunks == NULL) return -10; // no chunk data

    int chunk_count = LE(cert->chunk_hdr->count) & 0x7FFFFFFF;
    for (int i = 0; i < chunk_count; i++) {
        int r = verify_chunk_checksum(cert, i, fp, offset);
        if (r != 0)
            return r;
    }
    return 0;
}
