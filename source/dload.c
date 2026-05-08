#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "dload.h"
#include "device_connection.h"
#include "dload_constants.h"
#include "endian.h"

int dload_do_echo(uint8_t *buffer, size_t length)
{
    uint8_t *recv_buf = NULL;
    size_t out_size = 0;

    int r = 0;

    uint8_t *msg_buffer = (uint8_t *)malloc(0x8 + length);
    if (msg_buffer == NULL) {
        r = kDload_OutOfMemory;
        goto end;
    }

    *(uint32_t *)(msg_buffer + 0x0) = LE(BB0_ECHO);
    *(uint32_t *)(msg_buffer + 0x4) = LE(length);
    memcpy(msg_buffer + 0x8, buffer, length);

    r = dload_device_send_packet(msg_buffer, 0x8 + length);
    if (r < 0) goto end;

    r = dload_device_recv_packet(&recv_buf, &out_size);
    if (r < 0) goto end;

    r = kDload_Success;
    if (LE(*(uint32_t *)(recv_buf + 0x0)) != BB0_ECHO ||
        out_size != 0x8 + length)
        r = kDload_InvalidResponse;

end:
    if (msg_buffer != NULL)
        free(msg_buffer);
    if (recv_buf != NULL)
        free(recv_buf);
    return r;
}

int dload_get_version(dload_version_t *out_version)
{
    uint8_t *recv_buf = NULL;
    size_t out_size = 0;

    int r = 0;

    uint8_t *msg_buffer = (uint8_t *)malloc(0x8);
    if (msg_buffer == NULL) {
        r = kDload_OutOfMemory;
        goto end;
    }

    *(uint32_t *)(msg_buffer + 0x0) = LE(BB0_GET_VERSION);
    *(uint32_t *)(msg_buffer + 0x4) = LE(0);

    r = dload_device_send_packet(msg_buffer, 0x8);
    if (r < 0) goto end;

    r = dload_device_recv_packet(&recv_buf, &out_size);
    if (r < 0) goto end;

    dload_hdr_t *response = (dload_hdr_t *)recv_buf;
    r = kDload_Success;
    if (LE(response->msg_type) != BB0_REPLY_VERSION ||
        out_size != 0x18 ||
        LE(response->msg_length) != 0x10)
        r = kDload_InvalidResponse;
    
    memcpy(out_version, recv_buf + 0x8, 0x10);

end:
    if (msg_buffer != NULL)
        free(msg_buffer);
    if (recv_buf != NULL)
        free(recv_buf);
    return r;
}

int dload_reset()
{
    int r = 0;

    uint8_t *msg_buffer = (uint8_t *)malloc(0x8);
    if (msg_buffer == NULL) {
        r = kDload_OutOfMemory;
        goto end;
    }

    *(uint32_t *)(msg_buffer + 0x0) = LE(BB0_RESET);
    *(uint32_t *)(msg_buffer + 0x4) = LE(0);

    r = dload_device_send_packet(msg_buffer, 0x8);
    if (r < 0) goto end;

    // we don't need to check for a response as the device will power off
    // TODO(Emma): will libusb even be happy about this?
    r = kDload_Success;

end:
    if (msg_buffer != NULL)
        free(msg_buffer);
    return r;
}

int dload_power_off()
{
    int r = 0;

    uint8_t *msg_buffer = (uint8_t *)malloc(0x8);
    if (msg_buffer == NULL) {
        r = kDload_OutOfMemory;
        goto end;
    }

    *(uint32_t *)(msg_buffer + 0x0) = LE(BB0_POWER_OFF);
    *(uint32_t *)(msg_buffer + 0x4) = LE(0);

    r = dload_device_send_packet(msg_buffer, 0x8);
    if (r < 0) goto end;

    // we don't need to check for a response as the device will power off
    // TODO(Emma): will libusb even be happy about this?
    r = kDload_Success;

end:
    if (msg_buffer != NULL)
        free(msg_buffer);
    return r;
}
