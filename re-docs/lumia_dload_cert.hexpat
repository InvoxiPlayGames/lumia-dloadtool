// Nokia Lumia DLOAD .cert file format pattern (for WP7 devices)
// by Emma / InvoxiPlayGames, 2026
// 
// Changelog:
//    v0 - Initial pattern. Based only off Lumia 800 recovery images.

#pragma author InvoxiPlayGames
#pragma description Nokia DLOAD .cert file
#pragma endian little

struct info_header {
  be u32 version; // always 0x121
  be u32 size; // size of all data afterwards
};

struct target_info {
  u32 info_size;
  u32 target_type; // 0 = flash write, 1 = flash erase, 2 = file write; 3 might be file erase but unused
  u32 unk_0x4; // always 0x4?
  u32 unk_0x1; // always 0x1?
  if (target_type == 1 || target_type == 0) {
    u64 flash_start;
    u64 flash_size;
  } else if (target_type == 2) {
    u32 unused; // always 0x0? seems unused
    char filename[info_size - 0x10]; // filename to put in image3
  }
};

struct chunk_info_sha1 {
  u64 start;
  u64 end;
  u8 sha1sum[0x14];
};

struct chunk_info_sha256 {
  u64 start;
  u64 end;
  u8 sha256sum[0x20];
};

struct chunk_list {
  u32 size;
  if (size >= 8) {
    u32 hash_type; // 0x1 = SHA-1, 0x2 = SHA-256 (always 0x2)
    u32 count;
    if (hash_type == 0x2) {
      chunk_info_sha256 chunks[count];
    } else {
      chunk_info_sha1 chunks[count];
    }
  }
};

struct sign_info {
  u32 size; // 0x20 for SHA-1 type hashes, 0x2C for SHA-256
  u32 unused; // always 0x0? unchecked
  u32 key_id; // 0x71 = main key, 0x75 = unk (always 0x71)
  u32 hash_type; // 0x1 = SHA-1, 0x2 = SHA-256 (always SHA-1)
  u8 pub_key_hash[hash_type == 0x2 ? 0x20 : 0x14];
};

struct signature_data {
  u32 sig_type; // 0x1 = RSAES-PKCS1-v1_5, 0x2 = RSASSA-PSS (always SSA-PSS)
  u32 hash_type; // 0x1 = SHA-1, 0x2 = SHA-256 (always SHA-256)
  u32 sig_length; // always 0x100?
  u8 signature[sig_length];
};

struct cert_file {
  info_header info;
  u8 pub_key_sha1[0x14];

  // -- below is sent to the device (hex codes indicate what the dload function to fetch it takes)

  // 0x1 - magic
  u32 wp70_magic; // 'WP70' in little endian ('07PW')

  // 0x2 - maybe version number?
  u32 always_zero; // errors if this is not zero

  // 0x3 - build info? not checked by DLOAD
  u32 unk1; // either 0x98031573 (boot/osbl/adsp) or 0x98031568 (erase/amss)
  u32 timestamp;
  u32 timestamp_ms; // maybe?
  u32 unk2; // 0x9D6829 on rm-801 files, 0x994AA2 on rm-801 qcom, 0x9B1365 on rm-8

  // 0x4 - unknown? doesn't seem to be used
  u32 is_osbl_maybe; // 1 in osbl, 0 elsewhere

  // 0x5 - sign/encrypt flag? maybe? - non-zero errors in flash functions, goes down some crypto path in file functions
  u32 sign_enc_flag_maybe; // 0 in osbl/boot/erase, 1 in adsp/amss - something to do with signing/encryption?

  // 0x6 - flash stage? maybe? - non-zero is checked against flashing.bin?
  u32 flash_stage_maybe; // 0 in rm-801 qcom, 1 in osbl, 2 in boot, 3 in adsp, 4 in amss, 6 in erase

  // 0x7 - unknown? all zeroes, fixed length, doesn't seem to be used at all
  u8 unk[0x14];

  // 0x8 - target size, 0x9 - target info
  target_info target;

  // 0xA - chunks size, 0xB - chunk info
  chunk_list file_chunks;

  // 0xC - siginfo size, 0xD - sig info
  sign_info signature_info;

  // 0xE - signature
  signature_data signature;
};

cert_file cert @ 0x0;
