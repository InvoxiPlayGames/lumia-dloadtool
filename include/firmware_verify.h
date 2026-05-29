#ifndef FIRMWARE_VERIFY_H
#define FIRMWARE_VERIFY_H

#include <stdio.h>
#include "rm_cert.h"

int verify_firmware_checksum(rm_cert_t *cert, FILE *fp, long offset);
int verify_chunk_checksum(rm_cert_t *cert, int chunk_i, FILE *fp, long offset);

#endif // FIRMWARE_VERIFY_H
