// Nokia DLOAD .cert file format pattern (for WP7 devices)
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
  u32 target_type; // 0 = flash write, 1 = flash erase, 2 = file write
  u32 unk_0x4; // always 0x4?
  u32 unk_0x1; // always 0x1?
  if (target_type == 1 || target_type == 0) {
    u64 flash_start;
    u64 flash_size;
  } else if (target_type == 2) {
    u32 unk1; // always 0x0?
    char filename[info_size - 0x10];
  }
};

struct chunk_info {
  u64 start;
  u64 length;
  u8 sha256sum[0x20];
};

struct chunk_list {
  u32 size;
  if (size >= 8) {
    u32 type; // always 2?
    u32 count;
    chunk_info chunks[count];
  }
};

struct sign_info {
  u32 size; // 0x20
  u32 unk1; // always 0x0?
  u32 unk2; // always 0x71?
  u32 unk3; // always 0x1?
  u8 pub_key_sha1[0x14];
};

struct signature_data {
  u32 unk1; // always 0x2?
  u32 unk2; // always 0x2?
  u32 sig_size; // always 0x100?
  u8 signature[0x100];
};

struct cert_file {
  info_header info;
  u8 pub_key_sha1[0x14];
  u32 wp70_magic; // 'WP70' in little endian ('07PW')
  u32 unk_zero1; // same across three files, 0x00000000
  u32 unk1; // either 0x98031573 (boot/osbl/adsp) or 0x98031568 (erase/amss)
  u32 timestamp;
  u32 timestamp_ms; // maybe?
  u32 unk2; // the same across all files, 0x009D6829
  u32 unk3; // 1 in osbl, 0 elsewhere
  u32 unk4; // 0 in osbl/boot/erase, 1 in adsp/amss
  u64 flash_stage_maybe; // 1 in osbl, 2 in boot, 3 in adsp, 4 in amss, 6 in erase
  u64 unk_zero2; // always 0?
  u64 unk_zero3; // always 0?
  target_info target;
  chunk_list file_chunks;
  sign_info signature_info;
  signature_data signature;
};

cert_file cert @ 0x0;
