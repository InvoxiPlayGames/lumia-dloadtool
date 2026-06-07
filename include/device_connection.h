/*
    device_connection.h
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

#ifndef DEVICE_CONNECTION_H
#define DEVICE_CONNECTION_H

#include <stdlib.h>
#include <stdint.h>

typedef enum _dload_device_errors_t {
    // success status code is always 0
    kDlDev_Success = 0,
    // device descovery errors
    kDlDev_NoneFound = -1001,
    kDlDev_WPModeFound = -1002, // device in Windows Phone OS
    kDlDev_WPBLModeFound = -1003, // device in Windows Phone bootloader
    kDlDev_QCModeFound = -1004, // device in qualcomm bootloader
    kDlDev_MultipleFound = -1005,
    kDlDev_DeviceAlreadyOpen = -1006,
    // misc device errors
    kDlDev_DeviceNotOpen = -1020,
    kDlDev_MessageTimeout = -1021,
    // usb backend errors (libusb)
    kDlDev_LibusbError = -2000,
    // usb backend errors (WinUSB)
    kDlDev_WinUsb_InvalidHandle = -3000,
    kDlDev_WinUsb_BadDevice = -3001,
    kDlDev_WinUsb_ConfigErr = -3040,
    kDlDev_WinUsb_Unknown = -3099,
    // misc random errors
    kDlDev_OutOfMemory = -4444,
} dload_device_errors_t;

int dload_init_usb(void);
void dload_close_usb(void);

int dload_detect_device(void);
int dload_open_device(void);
int dload_close_device(void);

int dload_device_send_packet(const uint8_t *buffer, size_t buffer_len);
int dload_device_recv_packet(uint8_t **out_buffer, size_t *out_len);

#endif // DEVICE_CONNECTION_H
