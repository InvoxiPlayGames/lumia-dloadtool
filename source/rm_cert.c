#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "endian.h"
#include "rm_cert.h"

static void *safe_buffer_read(void *buffer, size_t buf_sz, size_t offset, size_t length) {
    if (offset > buf_sz)
        return NULL;
    if (length > buf_sz - offset)
        return NULL;
    return (void *)((uint8_t *)buffer + offset);
}

int rm_cert_from_buffer(void *buf, size_t buf_sz, rm_cert_t *out) {
    if (out == NULL || buf == NULL) return kRmC_InvalidArg;
    out->buf = buf;
    out->buf_sz = buf_sz;

    // let's skip over the first 0x14 bytes because that's the public key hash
    size_t curr_off = 0x14;
    out->device_buf = safe_buffer_read(buf, buf_sz, curr_off, buf_sz - curr_off);
    out->device_buf_sz = buf_sz - curr_off;
    if (out->device_buf == NULL)
        return kRmC_FailedBoundsCheck;
    
    // read the generic file info
    out->info = (rm_cert_static_info_t *)safe_buffer_read(buf, buf_sz, curr_off, sizeof(rm_cert_static_info_t));
    curr_off += sizeof(rm_cert_static_info_t);
    if (out->info == NULL)
        return kRmC_FailedBoundsCheck;
    // verify the generic file info
    if (LE(out->info->wp70_magic) != RMCERT_WP70_MAGIC)
        return kRmC_InvalidMagic;
    if (out->info->always_zero != 0)
        return kRmC_FailedValidation;
    static const uint8_t zerobuf[0x14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    if (memcmp(zerobuf, out->info->unk3, sizeof(zerobuf)) != 0)
        return kRmC_FailedValidation;
    
    // read the target info
    uint32_t *target_info_sz_p = (uint32_t *)safe_buffer_read(buf, buf_sz, curr_off, sizeof(uint32_t));
    if (target_info_sz_p == NULL)
        return kRmC_FailedBoundsCheck;
    curr_off += sizeof(uint32_t);
    uint32_t target_info_sz = LE(*target_info_sz_p);
    if (target_info_sz < sizeof(rm_cert_target_hdr_t))
        return kRmC_FailedBoundsCheck;
    out->target = (rm_cert_target_hdr_t *)safe_buffer_read(buf, buf_sz, curr_off, target_info_sz);
    if (out->target == NULL)
        return kRmC_FailedBoundsCheck;
    curr_off += target_info_sz;
    // verify the target info and its specific type
    if (LE(out->target->target_type) == kTargetFileWrite) {
        if (target_info_sz <= (sizeof(rm_cert_target_hdr_t) + 4)) // 4 bytes padding
            return kRmC_FailedBoundsCheck;
        out->file = (char *)safe_buffer_read(out->target, target_info_sz, sizeof(rm_cert_target_hdr_t) + 0x4, target_info_sz - (sizeof(rm_cert_target_hdr_t) + 4));
        if (out->file == NULL)
            return kRmC_FailedBoundsCheck;
    } else if (LE(out->target->target_type) == kTargetFlashWrite || LE(out->target->target_type) == kTargetFlashErase) {
        if (target_info_sz != (sizeof(rm_cert_target_hdr_t) + sizeof(rm_cert_target_flash_t)))
            return kRmC_FailedBoundsCheck;
        out->flash = (rm_cert_target_flash_t *)safe_buffer_read(out->target, target_info_sz, sizeof(rm_cert_target_hdr_t), sizeof(rm_cert_target_flash_t));
        if (out->flash == NULL)
            return kRmC_FailedBoundsCheck;
    } else {
        return kRmC_UnsupportedTarget;
    }

    // read the file chunks info
    uint32_t *chunks_info_sz_p = (uint32_t *)safe_buffer_read(buf, buf_sz, curr_off, sizeof(uint32_t));
    if (chunks_info_sz_p == NULL)
        return kRmC_FailedBoundsCheck;
    curr_off += sizeof(uint32_t);
    uint32_t chunks_info_sz = LE(*chunks_info_sz_p);
    // if the chunks info size is 0 then we can skip over it (e.g. flash erase)
    if (chunks_info_sz != 0) {
        // read the chunk info header
        if (chunks_info_sz < sizeof(rm_cert_chunk_hdr_t))
            return kRmC_FailedBoundsCheck;
        out->chunk_hdr = (rm_cert_chunk_hdr_t *)safe_buffer_read(buf, buf_sz, curr_off, chunks_info_sz);
        if (out->chunk_hdr == NULL)
            return kRmC_FailedBoundsCheck;
        curr_off += chunks_info_sz;
        // verify the chunk info header and get the list of chunks from the file
        if (LE(out->chunk_hdr->hash_type) == kHashSha1) {
            out->chunks = safe_buffer_read(out->chunk_hdr, chunks_info_sz, sizeof(rm_cert_chunk_hdr_t), sizeof(rm_cert_chunk_sha1_t) * LE(out->chunk_hdr->count));
            if (out->chunks == NULL)
                return kRmC_FailedBoundsCheck;
        } else if (LE(out->chunk_hdr->hash_type) == kHashSha256) {
            out->chunks = safe_buffer_read(out->chunk_hdr, chunks_info_sz, sizeof(rm_cert_chunk_hdr_t), sizeof(rm_cert_chunk_sha256_t) * LE(out->chunk_hdr->count));
            if (out->chunks == NULL)
                return kRmC_FailedBoundsCheck;
        } else {
            return kUnsupportedHashType;
        }
    }

    // read the signature info
    uint32_t *sig_info_sz_p = (uint32_t *)safe_buffer_read(buf, buf_sz, curr_off, sizeof(uint32_t));
    if (sig_info_sz_p == NULL)
        return kRmC_FailedBoundsCheck;
    curr_off += sizeof(uint32_t);
    uint32_t sig_info_sz = LE(*sig_info_sz_p);
    if (sig_info_sz < sizeof(rm_cert_sig_info_t))
        return kRmC_FailedBoundsCheck;
    out->sig_info = (rm_cert_sig_info_t *)safe_buffer_read(buf, buf_sz, curr_off, sig_info_sz);
    if (out->sig_info == NULL)
        return kRmC_FailedBoundsCheck;
    curr_off += sig_info_sz;
    // verify the signature info and get the public key hash out of it
    if (LE(out->sig_info->hash_type) == kHashSha1) {
        out->pub_key_hash = safe_buffer_read(out->sig_info, sig_info_sz, sizeof(rm_cert_sig_info_t), 0x14);
    } else if (LE(out->sig_info->hash_type) == kHashSha256) {
        out->pub_key_hash = safe_buffer_read(out->sig_info, sig_info_sz, sizeof(rm_cert_sig_info_t), 0x20);
    } else {
        return kUnsupportedHashType;
    }

    // read the signature header
    out->sig_hdr = (rm_cert_sig_hdr_t *)safe_buffer_read(buf, buf_sz, curr_off, sizeof(rm_cert_sig_hdr_t));
    if (out->sig_hdr == NULL)
        return kRmC_FailedBoundsCheck;
    curr_off += sizeof(rm_cert_sig_hdr_t);
    out->sig_data = safe_buffer_read(buf, buf_sz, curr_off, LE(out->sig_hdr->sig_size));
    if (out->sig_data == NULL)
        return kRmC_FailedBoundsCheck;

    return kRmC_Success;
}

void rm_cert_free(rm_cert_t *buf) {
    free(buf->buf);
    free(buf);
}

int rm_cert_from_file(FILE *fp, rm_cert_t **out) {
    if (fp == NULL || out == NULL)
        return kRmC_InvalidArg;
    // read the header from the certificate file
    rm_cert_hdr_t header;
    if (fread(&header, sizeof(rm_cert_hdr_t), 1, fp) != 1) {
        printf("failed to read header\n");
        return kRmC_FileReadFailed;
    }
    if (BE(header.version) != RMCERT_VERSION) {
        return kRmC_InvalidVersion;
    }
    // read the certificate data from file
    void *cert_data = malloc(BE(header.size));
    if (cert_data == NULL) {
        return kRmC_OutOfMemory;
    }
    if (fread(cert_data, 1, BE(header.size), fp) != BE(header.size)) {
        free(cert_data);
        printf("failed to read %i bytes\n", BE(header.size));
        return kRmC_FileReadFailed;
    }
    // parse the certificate data
    rm_cert_t *parsed = (rm_cert_t *)malloc(sizeof(rm_cert_t));
    if (parsed == NULL) {
        free(cert_data);
        return kRmC_OutOfMemory;
    }
    memset((void *)parsed, 0, sizeof(rm_cert_t));
    int r = rm_cert_from_buffer(cert_data, BE(header.size), parsed);
    if (r != kRmC_Success) {
        free(cert_data);
        free(parsed);
    } else {
        *out = parsed;
    }
    return r;
}
