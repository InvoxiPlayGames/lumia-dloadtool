#ifndef RM_CERT_H
#define RM_CERT_H

#include <stdint.h>

// !! Unlike everything else, this struct is in BIG ENDIAN
typedef struct _rm_cert_hdr_t {
    uint32_t version;
    uint32_t size;
} rm_cert_hdr_t;

typedef struct _rm_cert_chunk_s256_t {
    uint64_t start;
    uint64_t length;
    uint8_t sha256[0x20];
} rm_cert_chunk_s256_t;

typedef enum _rm_cert_target_e {
    kTargetFlashWrite = 0,
    kTargetFlashErase = 1,
    kTargetFileWrite = 2
} rm_cert_target_e;

typedef struct _rm_cert_flash_target_t {
    uint32_t target_type;
    uint32_t unk_0x4; // always 0x4
    uint32_t key_id;
    uint64_t flash_start;
    uint64_t flash_size;
} rm_cert_flash_target_t;

typedef struct _rm_cert_sig_info_t {
    uint32_t unk1; // always 0x0?
    uint32_t unk2; // always 0x71?
    uint32_t unk3; // always 0x1?
    uint8_t pub_key_sha1[0x14];
} rm_cert_sig_info_t;

typedef struct _rm_cert_sig_t {
    uint32_t unk1; // always 0x2?
    uint32_t unk2; // always 0x2?
    uint32_t sig_size; // always 0x100?
    uint8_t signature[0x100];
} rm_cert_sig_t;

typedef enum _rm_cert_stage_e {
    kStageOsbl = 1,
    kStageBoot = 2,
    kStageAdsp = 3,
    kStageAmss = 4,
    kStageErase = 6
} rm_cert_stage_e;

#define RMCERT_WP70_MAGIC 0x57503730

#endif // RM_CERT_H
