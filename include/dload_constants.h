/*
    dload_constants.h
    Command types and constant values used in DLOAD.
*/

#ifndef DLOAD_CONSTANTS_H
#define DLOAD_CONSTANTS_H

// generic "BB0" commands, self-explanatory
#define BB0_ECHO 0xbb0bb001
#define BB0_RESET 0xbb0bb002
#define BB0_GET_VERSION 0xbb0bb003
#define BB0_REPLY_VERSION 0xbb0bb004
#define BB0_POWER_OFF 0xbb0bb005 // seems to actually reset on some devices

// magic value for BB6 commands
#define BB6_MAGIC 0xbb6bb601
// static msg type value in the BB6 header
#define BB6_MSG_TYPE_REQ 0x2
#define BB6_MSG_TYPE_RES 0x3

// contains a log message
#define BB6_LOG_TLV 0x10001
// contains the actual file/flash data
#define BB6_DATA_TLV 0x10002
// sends one of the BB6_CONTROL messages
#define BB6_CONTROL_TLV 0x10003

// contains flash metadata for erase or secure write
#define BB6_IMAGE_TLV 0x20001
// contains file metadata for secure write
#define BB6_FILE_TLV 0x20002
// contains cert metadata
#define BB6_CERT_TLV 0x20003

// contains sequence number and command ID
#define BB6_REQUEST_TLV 0x30001
// contains sequence number and return code
#define BB6_RESPONSE_TLV 0x30002

// resets the device back into DLOAD mode
#define BB6_TYPE_RESET 0x1
// sets control settings (see BB6_CONTROL_TLV, BB6_CONTROL_*)
#define BB6_TYPE_CONTROL 0x3
// sends a certificate to the secure area
#define BB6_TYPE_CERT 0x4
// powers off the device
#define BB6_TYPE_POWEROFF 0x6
// writes to the flash / to a file based on BB6_IMAGE_TLV/BB6_FILE_TLV
#define BB6_TYPE_WRITE 0x100
// erases pages from flash
#define BB6_TYPE_ERASE 0x101
// starts/continues a delayed write to the flash
#define BB6_TYPE_WRITE_DELAYED 0x104
// flushes delayed writes
#define BB6_TYPE_FLUSH_DELAYED 0x105
// assuming this is similar to write delayed however it doesn't actually delay
#define BB6_TYPE_ERASE_DELAYED 0x106

// unlocks something or other i don't know, see the nokia TLVs
#define BB6_TYPE_SECRET_NOKIA_COMMAND 0x200
// gets information about a public key stored on the device
#define BB6_GET_NOKIA_KEY_TLV 0x40001
// send a signature signed with that public key to unlock a flag.
// no clue what this enables
#define BB6_SUBMIT_NOKIA_SIG_TLV 0x40002

// TODO: document
// definitely don't touch this one though it fucks with the modem partitions
// very high chance of either a brick or never able to cellular again
#define BB6_TYPE_MODEM_BACKUP_RESTORE 0x10000

// TODO: document
#define BB6_CONTROL_UNK1 0x10001
#define BB6_CONTROL_UNK2 0x10002
#define BB6_CONTROL_UNK3 0x10003
#define BB6_CONTROL_SET_TIMEOUT 0x10004
#define BB6_CONTROL_SET_TIMEOUT_ACTION 0x10005

#endif // DLOAD_CONSTANTS_H
