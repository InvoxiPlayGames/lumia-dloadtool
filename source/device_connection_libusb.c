/*
    device_connection_libusb.c
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

#ifndef _WIN32

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <libusb.h>

#include "device_connection.h"
#include "device_ids.h"

// make a global context in case we're a library in another application
static struct libusb_context *ctx;

// the DLOAD device we have a hold on
static libusb_device *lusb_device = NULL;
static libusb_device_handle *device_handle = NULL;

// 10 seconds is a lot.. but flashing can take a while!
#define DEVICE_TIMEOUT_MS 10000

int dload_init_usb(void)
{
    // init the libusb context
#if LIBUSB_API_VERSION >= 0x0100010A
    int r = libusb_init_context(&ctx, NULL, 0);
#else
    int r = libusb_init(&ctx);
#endif

    if (r != LIBUSB_SUCCESS)
        return kDlDev_LibusbError - -r;

    return kDlDev_Success;
}

void dload_close_usb(void)
{
    // close the usb library
    libusb_exit(ctx);
}

int dload_detect_device(void)
{
    // if we already have a device, report back and don't scan again
    if (lusb_device != NULL)
    {
        return kDlDev_DeviceAlreadyOpen;
    }
    libusb_device *detected_device = NULL;
    int num_seen_devices = 0;
    bool has_seen_qc_mode = false;
    bool has_seen_ms_mode = false;
    bool has_seen_msbl_mode = false;
    int r = kDlDev_NoneFound;
    // scan the list of devices
    libusb_device **device_list = NULL;
    ssize_t sz = libusb_get_device_list(ctx, &device_list);
    for (ssize_t i = 0; i < sz; i++)
    {
        // fetch the descriptor to see if it's a device in DLOAD mode
        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(device_list[i], &desc);
        if (desc.idVendor == NOKIA_VID && desc.idProduct == DLOAD_PID)
        {
            detected_device = libusb_ref_device(device_list[i]);
            num_seen_devices++;
            r = kDlDev_Success;
        }
        // check for other modes the device could be in
        else if (desc.idProduct == QCBL_PID && desc.idVendor == QUALCOMM_VID)
        {
            has_seen_qc_mode = true;
            num_seen_devices++;
        }
        else if (desc.idVendor == MICROSOFT_VID &&
            (desc.idProduct == WM7_MAINOS_PID || desc.idProduct == WM7_BLDR_PID || desc.idProduct == WM7_FFU_PID))
        {
            has_seen_msbl_mode = (desc.idProduct == WM7_BLDR_PID || desc.idProduct == WM7_FFU_PID);
            has_seen_ms_mode = true;
            num_seen_devices++;
        }
    }
    if (num_seen_devices > 1)
        r = kDlDev_MultipleFound;
    else if (has_seen_msbl_mode)
        r = kDlDev_WPBLModeFound;
    else if (has_seen_ms_mode)
        r = kDlDev_WPModeFound;
    else if (has_seen_qc_mode)
        r = kDlDev_QCModeFound;
    // if we aren't succeeding, destroy any device handle we might've made
    if (r != kDlDev_Success && detected_device != NULL)
    {
        libusb_unref_device(detected_device);
        detected_device = NULL;
    }
    else if (r == kDlDev_Success)
    {
        lusb_device = detected_device;
    }
    // free the list of devices in libusb
    libusb_free_device_list(device_list, 1);
    return r;
}

int dload_open_device(void)
{
    if (lusb_device == NULL)
        return kDlDev_NoneFound;

    int r = libusb_open(lusb_device, &device_handle);
    if (r != LIBUSB_SUCCESS)
        return kDlDev_LibusbError - -r;

    r = libusb_claim_interface(device_handle, 0);
    if (r != LIBUSB_SUCCESS)
        return kDlDev_LibusbError - -r;

    return kDlDev_Success;
}

int dload_close_device(void)
{
    // make sure we have a device open
    if (device_handle == NULL)
        return kDlDev_DeviceNotOpen;

    // close it
    libusb_close(device_handle);
    libusb_unref_device(lusb_device);
    device_handle = NULL;
    lusb_device = NULL;

    return kDlDev_Success;
}

int dload_device_send_packet(const uint8_t *buffer, size_t buffer_len)
{
    // make sure we have a device open
    if (device_handle == NULL)
        return kDlDev_DeviceNotOpen;

    // send the message
    int r = libusb_bulk_transfer(device_handle, 0x01, (uint8_t *)buffer, buffer_len, NULL, DEVICE_TIMEOUT_MS);

    if (r == LIBUSB_ERROR_TIMEOUT)
        return kDlDev_MessageTimeout;
    else if (r < 0)
        return kDlDev_LibusbError - -r;

    return kDlDev_Success;
}

int dload_device_recv_packet(uint8_t **out_buffer, size_t *out_len)
{
    // make sure we have a device open
    if (device_handle == NULL)
        return kDlDev_DeviceNotOpen;

    // allocate a buffer for our message
    void *full_msg = malloc(0x2020);
    if (full_msg == NULL)
        return kDlDev_OutOfMemory;

    *out_buffer = (uint8_t *)full_msg;
    int transferred_bytes = 0;
    // TODO(Emma): does the max output size (0x2020) change? will certain requests take longer + need a longer timeout?
    int r = libusb_bulk_transfer(device_handle, 0x81, full_msg, 0x2020, &transferred_bytes, DEVICE_TIMEOUT_MS);

    if (r == 0)
        *out_len = transferred_bytes;
    else if (r == LIBUSB_ERROR_TIMEOUT)
        return kDlDev_MessageTimeout;
    else if (r < 0)
        return kDlDev_LibusbError - -r;

    return kDlDev_Success;
}

#endif // _WIN32
