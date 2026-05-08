#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <strings.h>

#include "bb6_msg_parser.h"
#include "bb6_msg_builder.h"
#include "bb6_tlv.h"
#include "dload_constants.h"

#include "dload.h"
#include "device_connection.h"

void hexdump(const void *buf, size_t len)
{
    if (buf == NULL || len == 0)
        return;

    for (size_t i = 0; i < len; i++)
        printf("%02x", ((uint8_t *)buf)[i]);

    printf("\n");
}

void test_bb0_msgs()
{
    int r = 0;

    uint8_t echo[] = {'H', 'A', 'W', 'K', '2'};
    r = dload_do_echo(echo, sizeof(echo));
    if (r != kDload_Success) {
        printf("unknown error sending echo: %i\n", r);
        return;
    }
    printf("echo success\n");

    dload_version_t ver;
    r = dload_get_version(&ver);
    if (r != kDload_Success) {
        printf("unknown error fetching version: %i\n", r);
        return;
    }
    printf("DLOAD build timestamp: %i\n", LE(ver.timestamp));
    printf("DLOAD version data: ");
    hexdump(ver.unk, sizeof(ver.unk));
}

void test_bb6_msgs(int key_id, int print_key_id)
{
    int r = 0;
    
    // build the key get request
    bb6_msg_inprog_t *newmsg = bb6_alloc_request(BB6_TYPE_SECRET_NOKIA_COMMAND);
    if (newmsg == NULL) {
        printf("error allocating BB6 request\n");
        return;
    }
    BB6_GET_NOKIA_KEY_TLV_t nokeyget = {
        .key_id = LE(key_id),
        .unk = LE(0x4)
    };
    r = bb6_add_tlv(newmsg, BB6_GET_NOKIA_KEY_TLV, &nokeyget, sizeof(BB6_GET_NOKIA_KEY_TLV_t));
    if (r != 0) {
        printf("error adding data to request\n");
        return;
    }

    // serialize the message
    size_t bb6byteslen = 0;
    uint8_t *bb6bytes = bb6_serialize_request(newmsg, &bb6byteslen);
    if (bb6bytes == NULL) {
        printf("error serializing BB6 request\n");
        return;
    }

    // send it off - we don't need the request anymore
    r = dload_device_send_packet(bb6bytes, bb6byteslen);
    bb6_free_request(newmsg);
    free(bb6bytes);

    if (r != kDlDev_Success) {
        printf("unknown error sending key request: %i\n", r);
        return;
    }

    // get the response from the device
    uint8_t *recvbuf = NULL;
    size_t recvlen = 0;
    r = dload_device_recv_packet(&recvbuf, &recvlen);
    if (r != kDlDev_Success) {
        printf("unknown error sending key request: %i\n", r);
        return;
    }

    int rcode = bb6_get_return_code(recvbuf, recvlen);
    if (rcode == -1)  {
        printf("failed to get response code from key response\n");
        free(recvbuf);
        return;
    }
    if (rcode != 0)
        printf("return code: 0x%x\n", rcode);

    size_t resize = 0;
    const BB6_RESPONSE_NOKIA_KEY_TLV_t *respthing = bb6_get_packet_tlv(recvbuf, recvlen, BB6_DATA_TLV, &resize);
    if (respthing == NULL || resize != sizeof(BB6_RESPONSE_NOKIA_KEY_TLV_t)) {
        printf("failed to get key response\n");
        free(recvbuf);
        return;
    }

    printf("0x%x: ", print_key_id);
    hexdump(respthing->hash, sizeof(respthing->hash));

    // free the response
    free(recvbuf);
}

void do_tests()
{
    test_bb0_msgs();

    // also doubles at poking at the undocumented secret interface
    test_bb6_msgs(2, 0x72);
    test_bb6_msgs(1, 0x73);
    test_bb6_msgs(3, 0x74);

    printf("resetting device\n");
    int r = dload_reset();
    if (r != kDload_Success)
    {
        printf("unknown error resetting device: %i\n", r);
        return;
    }
}

int main(int argc, char **argv)
{
    // initialise the usb library
    dload_init_usb();

    // detect device
    int r = dload_detect_device();
    if (r == kDlDev_Success)
        printf("successfully detected DLOAD device!\n");
    else if (r == kDlDev_NoneFound)
        printf("no devices detected\n");
    else if (r == kDlDev_DeviceAlreadyOpen)
        printf("a DLOAD device is already open\n");
    else if (r == kDlDev_MultipleFound)
        printf("several devices were detected; please only connect one\n");
    else if (r == kDlDev_QCModeFound)
        printf("a device in Qualcomm mode was found, this is only compatible with DLOAD bootloaders\n");
    else if (r == kDlDev_WPModeFound)
        printf("device in normal mode found, please put it into DLOAD mode\n");
    else if (r == kDlDev_WPBLModeFound)
        printf("device in the WP7 bootloader found, please put it into DLOAD mode\n");
    else
        printf("unknown error detecting device: %i\n", r);

    // device detection failed
    if (r != kDlDev_Success)
        return -1;

    // actually open the device
    r = dload_open_device();
    if (r != kDlDev_Success)
    {
        printf("unknown error opening device: %i\n", r);
        return -2;
    }

    if (argc >= 2 && strcasecmp(argv[1], "test") == 0) {
        do_tests();
    } else {
        dload_version_t ver;
        r = dload_get_version(&ver);
        if (r != kDload_Success)
            printf("failed to get version: error %i\n", r);
        else
            printf("DLOAD build timestamp: %i\n", LE(ver.timestamp));
    }

    // terminate the usb library
    dload_close_usb();
    return 0;
}
