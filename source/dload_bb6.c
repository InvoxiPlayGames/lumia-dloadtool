/*
    dload_bb6.c
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "dload.h"
#include "device_connection.h"
#include "dload_constants.h"
#include "bb6_tlv.h"
#include "bb6_msg_builder.h"
#include "bb6_msg_parser.h"
#include "endian.h"
#include "rm_cert.h"

// TODO(Emma): reduce the amount of duplicated code here

int dload_do_control(uint32_t ctrl_reg, uint32_t ctrl_val)
{
    int r = 0;
    // build the request
    bb6_msg_inprog_t *newmsg = bb6_alloc_request(BB6_TYPE_CONTROL);
    if (newmsg == NULL) {
        printf("%s: error allocating BB6 request\n", __func__);
        return -1;
    }

    // add the control msg
    BB6_CONTROL_TLV_t ctrl = {
        .control_id = LE(ctrl_reg),
        .length = LE(4),
        .value = LE(ctrl_val)
    };
    r = bb6_add_tlv(newmsg, BB6_CONTROL_TLV, &ctrl, sizeof(BB6_CONTROL_TLV_t));
    if (r != 0) {
        printf("%s: error adding metadata to the request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }

    // serialize the message
    size_t bb6byteslen = 0;
    uint8_t *bb6bytes = bb6_serialize_request(newmsg, &bb6byteslen);
    if (bb6bytes == NULL) {
        printf("%s: error serializing BB6 request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }

    // send it off - we don't need the request anymore
    r = dload_device_send_packet(bb6bytes, bb6byteslen);
    bb6_free_request(newmsg);
    free(bb6bytes);
    if (r != kDlDev_Success) {
        printf("%s: unknown error sending request (%i)\n", __func__, r);
        return r;
    }

    // get the response from the device
    uint8_t *recvbuf = NULL;
    size_t recvlen = 0;
    r = dload_device_recv_packet(&recvbuf, &recvlen);
    if (r != kDlDev_Success) {
        printf("%s: unknown error getting response (%i)\n", __func__, r);
        return r;
    }

    int rcode = bb6_get_return_code(recvbuf, recvlen);
    if (rcode == -1)  {
        printf("%s: invalid response from device\n", __func__);
        free(recvbuf);
        return -1;
    }
    if (rcode != 0) {
        printf("%s: device returned error code (0x%x)\n", __func__, rcode);
        free(recvbuf);
        return rcode;
    }

    // write successful
    free(recvbuf);
    return 0;
}

int dload_send_cert(rm_cert_t *cert)
{
    if (cert == NULL || cert->device_buf == NULL || cert->device_buf_sz == 0) {
        printf("%s: valid certificate not provided\n", __func__);
        return -1;
    }

    int r = 0;
    // build the request
    bb6_msg_inprog_t *newmsg = bb6_alloc_request(BB6_TYPE_CERT);
    if (newmsg == NULL) {
        printf("%s: error allocating BB6 request\n", __func__);
        return -1;
    }

    // add the flash write metadata
    BB6_CERT_TLV_t metadata = {
        .length = LE(cert->device_buf_sz),
        .unk = LE(0),
        .key_id = LE(1) // TODO(Emma): get these values from the cert itself
    };
    r = bb6_add_tlv(newmsg, BB6_CERT_TLV, &metadata, sizeof(BB6_CERT_TLV_t));
    if (r != 0) {
        printf("%s: error adding metadata to the request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }

    // add the certificate data
    r = bb6_add_tlv(newmsg, BB6_DATA_TLV, cert->device_buf, cert->device_buf_sz);
    if (r != 0) {
        printf("%s: error adding data to the request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }

    // serialize the message
    size_t bb6byteslen = 0;
    uint8_t *bb6bytes = bb6_serialize_request(newmsg, &bb6byteslen);
    if (bb6bytes == NULL) {
        printf("%s: error serializing BB6 request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }

    // send it off - we don't need the request anymore
    r = dload_device_send_packet(bb6bytes, bb6byteslen);
    bb6_free_request(newmsg);
    free(bb6bytes);
    if (r != kDlDev_Success) {
        printf("%s: unknown error sending request (%i)\n", __func__, r);
        return r;
    }

    // get the response from the device
    uint8_t *recvbuf = NULL;
    size_t recvlen = 0;
    r = dload_device_recv_packet(&recvbuf, &recvlen);
    if (r != kDlDev_Success) {
        printf("%s: unknown error getting response (%i)\n", __func__, r);
        return r;
    }

    int rcode = bb6_get_return_code(recvbuf, recvlen);
    if (rcode == -1)  {
        printf("%s: invalid response from device\n", __func__);
        free(recvbuf);
        return -1;
    }
    if (rcode != 0) {
        printf("%s: device returned error code (0x%x)\n", __func__, rcode);
        free(recvbuf);
        return rcode;
    }

    // cert delivery successful
    free(recvbuf);
    return 0;
}

int dload_flash_write_delayed(uint64_t address, uint64_t offset, void *data, size_t length)
{   
    int r = 0;
    // build the request
    bb6_msg_inprog_t *newmsg = bb6_alloc_request(BB6_TYPE_WRITE_DELAYED);
    if (newmsg == NULL) {
        printf("%s: error allocating BB6 request\n", __func__);
        return -1;
    }

    // add the flash write metadata
    BB6_IMAGE_TLV_t metadata = {
        .size = LE64(length),
        .addr = LE64(address),
        .offset = LE64(offset),
        .unk = LE(4),
        .key_id = LE(1)
    };
    r = bb6_add_tlv(newmsg, BB6_IMAGE_TLV, &metadata, sizeof(BB6_IMAGE_TLV_t));
    if (r != 0) {
        printf("%s: error adding metadata to the request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }

    // add the image data
    r = bb6_add_tlv(newmsg, BB6_DATA_TLV, data, length);
    if (r != 0) {
        printf("%s: error adding data to the request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }

    // serialize the message
    size_t bb6byteslen = 0;
    uint8_t *bb6bytes = bb6_serialize_request(newmsg, &bb6byteslen);
    if (bb6bytes == NULL) {
        printf("%s: error serializing BB6 request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }

    // send it off - we don't need the request anymore
    r = dload_device_send_packet(bb6bytes, bb6byteslen);
    bb6_free_request(newmsg);
    free(bb6bytes);
    if (r != kDlDev_Success) {
        printf("%s: unknown error sending request (%i)\n", __func__, r);
        return r;
    }

    // get the response from the device
    uint8_t *recvbuf = NULL;
    size_t recvlen = 0;
    r = dload_device_recv_packet(&recvbuf, &recvlen);
    if (r != kDlDev_Success) {
        printf("%s: unknown error getting response (%i)\n", __func__, r);
        return r;
    }

    int rcode = bb6_get_return_code(recvbuf, recvlen);
    if (rcode == -1)  {
        printf("%s: invalid response from device\n", __func__);
        free(recvbuf);
        return -1;
    }
    if (rcode != 0) {
        printf("%s: device returned error code (0x%x)\n", __func__, rcode);
        free(recvbuf);
        return rcode;
    }

    // write successful
    free(recvbuf);
    return 0;
}

int dload_flush_delayed(void)
{
    int r = 0;
    // build the request
    bb6_msg_inprog_t *newmsg = bb6_alloc_request(BB6_TYPE_FLUSH_DELAYED);
    if (newmsg == NULL) {
        printf("%s: error allocating BB6 request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }
    // serialize the message
    size_t bb6byteslen = 0;
    uint8_t *bb6bytes = bb6_serialize_request(newmsg, &bb6byteslen);
    if (bb6bytes == NULL) {
        printf("%s: error serializing BB6 request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }

    // send it off - we don't need the request anymore
    r = dload_device_send_packet(bb6bytes, bb6byteslen);
    bb6_free_request(newmsg);
    free(bb6bytes);
    if (r != kDlDev_Success) {
        printf("%s: unknown error sending request (%i)\n", __func__, r);
        return r;
    }

    // get the response from the device
    uint8_t *recvbuf = NULL;
    size_t recvlen = 0;
    r = dload_device_recv_packet(&recvbuf, &recvlen);
    if (r != kDlDev_Success) {
        printf("%s: unknown error getting response (%i)\n", __func__, r);
        return r;
    }

    int rcode = bb6_get_return_code(recvbuf, recvlen);
    if (rcode == -1)  {
        printf("%s: invalid response from device\n", __func__);
        free(recvbuf);
        return -1;
    }
    if (rcode != 0) {
        printf("%s: device returned error code (0x%x)\n", __func__, rcode);
        free(recvbuf);
        return rcode;
    }

    // flush successful
    free(recvbuf);
    return 0;
}

int dload_reset_osbl(void)
{
    int r = 0;
    // build the request
    bb6_msg_inprog_t *newmsg = bb6_alloc_request(BB6_TYPE_RESET);
    if (newmsg == NULL) {
        printf("%s: error allocating BB6 request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }
    // serialize the message
    size_t bb6byteslen = 0;
    uint8_t *bb6bytes = bb6_serialize_request(newmsg, &bb6byteslen);
    if (bb6bytes == NULL) {
        printf("%s: error serializing BB6 request\n", __func__);
        bb6_free_request(newmsg);
        return -1;
    }

    // send it off - we don't need the request anymore
    r = dload_device_send_packet(bb6bytes, bb6byteslen);
    bb6_free_request(newmsg);
    free(bb6bytes);
    if (r != kDlDev_Success) {
        printf("%s: unknown error sending request (%i)\n", __func__, r);
        return r;
    }

    return 0;
}
