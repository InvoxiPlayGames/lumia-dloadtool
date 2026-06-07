/*
    firmware_flash.c
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

#include "dload.h"
#include "endian.h"
#include "rm_cert.h"

// TODO(Emma): use system SHA256 libraries when available rather than using existing code
#include "sha256.h"

static int flash_firmware_chunk(rm_cert_t *cert, int chunk_i, FILE *fp, long offset)
{
    static const uint32_t subchunk_sz = 0x20000;

    if (cert->chunk_hdr == NULL || cert->chunks == NULL) return -10; // no chunk data
    
    int total_chunk_count = LE(cert->chunk_hdr->count) & 0x7FFFFFFF;
    if (chunk_i >= total_chunk_count) return -2; // invalid chunk

    if (LE(cert->chunk_hdr->hash_type) == kHashSha256)
    {
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
        // compare to original
        if (memcmp(chunk->sha256, s256_out, 0x20) != 0) {
            printf("invalid checksum for chunk %i\n", chunk_i);
            free(chunk_data);
            return -5; // invalid checksum
        }

        // okay, now we split that into sub-chunks and send it to the device
        //  scary scary... five nights and freddy and whatnot..
        long remaining_sz = chunk_size;
        long data_offset = 0;
        uint8_t *chunk_data_u8 = (uint8_t *)chunk_data;
        while (remaining_sz > 0)
        {
            long send_sz = remaining_sz >= subchunk_sz ? subchunk_sz : remaining_sz;

            int r = dload_flash_write_delayed(LE64(cert->flash->flash_start) + LE(chunk->start), data_offset, chunk_data_u8+data_offset, send_sz);
            if (r != 0) {
                if (r > 0)
                    printf("device error when sending subchunk to device (0x%x)\n", r);
                else
                    printf("error when sending subchunk to device (%i)\n", r);
                free(chunk_data);
                return r; // failed to write flash chunk
            }

            data_offset += send_sz;
            remaining_sz -= send_sz;
        }

        // if we've gotten this far... commit the write to flash. SCARY SCARY!!! TOMAS!!!
        int r = dload_flush_delayed();
        if (r != 0) {
            if (r > 0)
                printf("device error when committing write to flash (0x%x)\n", r);
            else
                printf("error when committing write to flash (%i)\n", r);
            free(chunk_data);
            return -7; // failed to commit chunk to flash
        }
        free(chunk_data);
        return 0;
    }
    else
    {
        return -1; //unsupported hash type
    }

    return 0;
}

int flash_firmware(rm_cert_t *cert, FILE *fp, long offset)
{
    int r = 0;

    // verify certificate is correct
    if (LE(cert->target->target_type) != kTargetFlashWrite)
    {
        printf("unsupported target type in certificate file\n");
        return -9; // unsupported target
    }

    // send certificate data to device
    r = dload_send_cert(cert);
    if (r != 0) {
        if (r > 0)
            printf("device error when sending certificate (0x%x)\n", r);
        else
            printf("error when sending certificate (%i)\n", r);
        return r; // error sending cert
    }

    printf("starting flash..");
    fflush(stdout);

    if (cert->chunk_hdr == NULL || cert->chunks == NULL) return -10; // no chunk data
    int chunk_count = LE(cert->chunk_hdr->count) & 0x7FFFFFFF;
    for (int i = 0; i < chunk_count; i++) {
        int r = flash_firmware_chunk(cert, i, fp, offset);
        if (r != 0) 
        {
            // if we already wrote a chunk
            if (i >= 1) {
                printf("!! an error occurred mid-flash !!\n");
                printf("the device may be in an unusual or unusable state\n");
            }
            return r;
        }
        printf(".");
        fflush(stdout);
    }
    printf("done!\n");
    fflush(stdout);

    return 0;
}
