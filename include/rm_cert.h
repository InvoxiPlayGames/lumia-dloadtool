/*
    rm_cert.h
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

    --

    The structure definitions from this file - ones that read from original
    .cert files - can be used under the public domain.
*/

#ifndef RM_CERT_H
#define RM_CERT_H

#include <stdint.h>
#include <stdio.h>

// !! Unlike everything else, this struct is in BIG ENDIAN
#define RMCERT_VERSION 0x121
typedef struct _rm_cert_hdr_t {
    uint32_t version;
    uint32_t size;
} rm_cert_hdr_t;

typedef enum _rm_cert_hash_e {
    kHashSha1 = 0x1,
    kHashSha256 = 0x2
} rm_cert_hash_e;

typedef enum _rm_cert_sig_e {
    kSigRSAES_PKCS1_15 = 0x1,
    kSigRSASSA_PSS = 0x2
} rm_cert_sig_e;

typedef struct _rm_cert_chunk_hdr_t {
    uint32_t hash_type;
    uint32_t count;
} rm_cert_chunk_hdr_t;

typedef struct _rm_cert_chunk_sha256_t {
    uint64_t start;
    uint64_t end;
    uint8_t sha256[0x20];
} rm_cert_chunk_sha256_t;

typedef struct _rm_cert_chunk_sha1_t {
    uint64_t start;
    uint64_t end;
    uint8_t sha1[0x14];
} rm_cert_chunk_sha1_t;

typedef enum _rm_cert_target_e {
    kTargetFlashWrite = 0,
    kTargetFlashErase = 1,
    kTargetFileWrite = 2
} rm_cert_target_e;

typedef struct _rm_cert_target_hdr_t {
    uint32_t target_type;
    uint32_t unk_0x4;
    uint32_t unk_0x1;
} rm_cert_target_hdr_t;

typedef struct _rm_cert_target_flash_t {
    uint64_t flash_start;
    uint64_t flash_size;
} rm_cert_target_flash_t;

typedef struct _rm_cert_sig_info_t {
    uint32_t unused;
    uint32_t key_id;
    uint32_t hash_type;
    // followed by either SHA-1 (0x14) or SHA-256 (0x20) for hash of public key
} rm_cert_sig_info_t;

typedef struct _rm_cert_sig_hdr_t {
    uint32_t sig_type;
    uint32_t hash_type;
    uint32_t sig_size;
    // followed by signature of sig_size
} rm_cert_sig_hdr_t;

typedef enum _rm_cert_stage_e {
    kStageOsbl = 1,
    kStageBoot = 2,
    kStageAdsp = 3,
    kStageAmss = 4,
    kStageErase = 6
} rm_cert_stage_e;

typedef struct _rm_cert_static_info_t {
    uint32_t wp70_magic;

    uint32_t always_zero;

    uint32_t unk1;
    uint32_t timestamp;
    uint32_t timestamp_ms; // unlikely?
    uint32_t unk2;

    uint32_t is_osbl_maybe;

    uint32_t sign_enc_flag_maybe;

    uint32_t flash_stage_maybe;

    uint8_t unk3[0x14];
} rm_cert_static_info_t;

#define RMCERT_WP70_MAGIC 0x57503730

// structure as parsed by the functions in rm_cert.c 
typedef struct _rm_cert_t {
    // the buffer the cert actually gets loaded into, only for freeing
    void *buf;
    size_t buf_sz;
    // the buffer that you'd send to the device
    void *device_buf;
    size_t device_buf_sz;
    // the static info at the start of the certificate file
    rm_cert_static_info_t *info;
    // the target information
    rm_cert_target_hdr_t *target;
    rm_cert_target_flash_t *flash;
    const char *file;
    // the chunk info
    rm_cert_chunk_hdr_t *chunk_hdr;
    void *chunks;
    // the signature info
    rm_cert_sig_info_t *sig_info;
    void *pub_key_hash;
    // the signature
    rm_cert_sig_hdr_t *sig_hdr;
    void *sig_data;
} rm_cert_t;

// return codes
typedef enum _rm_cert_err_e {
    kRmC_Success = 0,
    kRmC_InvalidArg = -1,
    kRmC_FileReadFailed = -2,
    kRmC_InvalidVersion = -3,
    kRmC_FailedBoundsCheck = -4,
    kRmC_OutOfMemory = -5,
    kRmC_InvalidMagic = -6,
    kRmC_FailedValidation = -7,
    kRmC_UnsupportedTarget = -8,
    kRmC_UnsupportedHashType = -9,
} rm_cert_err_e;

int rm_cert_from_buffer(void *buf, size_t buf_sz, rm_cert_t *out);
int rm_cert_from_file(FILE *fp, rm_cert_t **out);
void rm_cert_free(rm_cert_t *buf);

#endif // RM_CERT_H
