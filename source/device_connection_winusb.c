/*
    device_connection_winusb.c
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

#ifdef _WIN32

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <winusb.h>
#include <cfgmgr32.h>
#include <initguid.h>

#include "device_connection.h"
#include "device_ids.h"

// "Care Suite USB Connectivity" WinUSB drivers, installed by WDRT and NCS, used for DLOAD
DEFINE_GUID(GUID_DEVINTERFACE_CareSuiteUSB,
    0x0FD3B15C,0xD457,0x45d8,0xA7,0x79,0xC2,0xB2,0xC9,0xF9,0xD0,0xFD);

// WM Zune Serial USB WinUSB drivers, installed by Zune app, used for DTPT
DEFINE_GUID(GUID_DEVINTERFACE_WMZuneSerUSB,
    0x246F9A1F,0x49EF,0x472b,0x89,0x2D,0xE4,0x43,0x3F,0x87,0xB0,0x03);

// Zune MTP USB WinUSB drivers, installed by Zune app, used for MTP
DEFINE_GUID(GUID_DEVINTERFACE_ZuneMTPZUSB,
    0xCA3D7387,0xF67B,0x11DA,0xBB,0xEC,0x80,0x00,0x60,0x0F,0xE8,0x00);

// the DLOAD device we have a hold on
static HANDLE hDeviceHandle = INVALID_HANDLE_VALUE;
static WINUSB_INTERFACE_HANDLE hWinusbHandle = INVALID_HANDLE_VALUE;

// 10 seconds is a lot.. but flashing can take a while!
#define DEVICE_TIMEOUT_MS 10000

static int winusb_err_convert(DWORD err)
{
    switch (err) {
        case ERROR_INVALID_HANDLE:
            return kDlDev_WinUsb_InvalidHandle;
        case ERROR_NOT_ENOUGH_MEMORY:
            return kDlDev_OutOfMemory;
        case ERROR_BAD_DEVICE:
            return kDlDev_WinUsb_BadDevice;
        case ERROR_SEM_TIMEOUT:
            return kDlDev_MessageTimeout;
        default:
            printf("Unknown error: 0x%08x\n", err);
            return HRESULT_FROM_WIN32(err);
    }
}

static inline bool hex_to_uint16(const char *str, uint16_t *out)
{
    if (out == NULL) return false;
    if (strlen(str) < 4) return false;
    *out = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t nibble = 0x0;
        if (str[i] >= '0' && str[i] <= '9') nibble = str[i] - '0';
        else if (str[i] >= 'A' && str[i] <= 'F') nibble = (str[i] - 'A') + 0xA;
        else if (str[i] >= 'a' && str[i] <= 'f') nibble = (str[i] - 'a') + 0xA;
        else return false;
        *out |= nibble << ((3 - i) * 4);
    }
    return true;
}

static int winusb_to_vidpid(const char *string, uint16_t *vid, uint16_t *pid)
{
#define WUSB_TEMPLATE "\\\\?\\USB#VID_....&PID_...."
    const char *template = WUSB_TEMPLATE;
    // make sure the provided string is at least the size of the template
    if (strlen(string) < sizeof(WUSB_TEMPLATE)-1)
        return -1;
    // make sure the string matches the template, with . acting as a filler character
    for (int i = 0; i < sizeof(WUSB_TEMPLATE)-1; i++)
        if (string[i] != template[i] && template[i] != '.')
            return -2;
    // parse the hex
    uint16_t vidwork = 0;
    uint16_t pidwork = 0;
    if (!hex_to_uint16(string + 12, &vidwork)) return -3;
    if (!hex_to_uint16(string + 21, &pidwork)) return -4;
    // output vidpid
    if (vid != NULL)
        *vid = vidwork;
    if (pid != NULL)
        *pid = pidwork;
#undef WUSB_TEMPLATE
    return 0;
}

static bool is_any_device_of_type_present(LPGUID lpGuid)
{
    CONFIGRET cr = CR_SUCCESS;
    ULONG     DeviceInterfaceListLength = 0;
    cr = CM_Get_Device_Interface_List_SizeA(&DeviceInterfaceListLength, lpGuid, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
    if (cr != CR_SUCCESS)
        return false; // assume not
    return DeviceInterfaceListLength > 2;
}

int dload_init_usb(void)
{
    // WinUSB doesn't need this
    return kDlDev_Success;
}

void dload_close_usb(void)
{
    // WinUSB doesn't need this
}

int dload_detect_device(void)
{
    // if we already have a device, report back and don't scan again
    if (hDeviceHandle != INVALID_HANDLE_VALUE)
        return kDlDev_DeviceAlreadyOpen;
    
    int r = kDlDev_NoneFound;

    bool has_seen_ms_mode = is_any_device_of_type_present((LPGUID)&GUID_DEVINTERFACE_WMZuneSerUSB);
    if (has_seen_ms_mode)
        r = kDlDev_WPModeFound;

    // TODO(Emma): detect devices present in the other modes (only missing is BLDR, mass storage)
    //             probably have to use setupapi rather than configmgr
    // begin WinUsb sample code
    CONFIGRET cr = CR_SUCCESS;
    DWORD     wr = 0;
    PSTR      DeviceInterfaceList = NULL;
    ULONG     DeviceInterfaceListLength = 0;

    // Enumerate all devices exposing the interface. Do this in a loop
    // in case a new interface is discovered while this code is executing,
    // causing CM_Get_Device_Interface_List to return CR_BUFFER_SMALL
    do {
        cr = CM_Get_Device_Interface_List_SizeA(&DeviceInterfaceListLength, (LPGUID)&GUID_DEVINTERFACE_CareSuiteUSB, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

        if (cr != CR_SUCCESS) {
            break;
        }

        DeviceInterfaceList = (PSTR)malloc(DeviceInterfaceListLength);

        if (DeviceInterfaceList == NULL) {
            return kDlDev_OutOfMemory;
        }

        cr = CM_Get_Device_Interface_ListA((LPGUID)&GUID_DEVINTERFACE_CareSuiteUSB, NULL, DeviceInterfaceList, DeviceInterfaceListLength, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

        if (cr != CR_SUCCESS) {
            free(DeviceInterfaceList);
        }
    } while (cr == CR_BUFFER_SMALL);

    // TODO(Emma): find out what error codes are meaningful from configmgr
    if (cr != CR_SUCCESS) {
        printf("ConfigMgr Error: 0x%08x\n", cr);
        r = kDlDev_WinUsb_ConfigErr - -cr;
        goto end;
    }

    // device found?
    if (DeviceInterfaceList[0] != 0) {
        // detect if we have several devices
        if (DeviceInterfaceListLength > strlen(DeviceInterfaceList) + 2)
            r = kDlDev_MultipleFound;
        if (has_seen_ms_mode) 
            r = kDlDev_MultipleFound;
        
        if (r != kDlDev_NoneFound) goto end;
        
        // make sure it's actually in DLOAD mode and not another care suite device
        uint16_t vid = 0, pid = 0;
        winusb_to_vidpid(DeviceInterfaceList, &vid, &pid); // TODO(Emma): should probably check err result here
        if (vid == NOKIA_VID && pid == DLOAD_PID) {
            hDeviceHandle = CreateFileA(DeviceInterfaceList, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
            if (hDeviceHandle != NULL) {
                r = kDlDev_Success;
            } else {
                r = winusb_err_convert(GetLastError());
            }
        } else if (vid == MICROSOFT_VID) {
            // care suite USB connectivity can have microsoft device IDs show up
            if (pid == WM7_BLDR_PID || pid == WM7_FFU_PID)
                r = kDlDev_WPBLModeFound;
            else if (pid == WM7_MAINOS_PID)
                r = kDlDev_WPModeFound;
        }
    }

end:
    if (DeviceInterfaceList != NULL)
        free(DeviceInterfaceList);
    return r;
}

int dload_open_device(void)
{
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
        return kDlDev_NoneFound;

    BOOL bResult = WinUsb_Initialize(hDeviceHandle, &hWinusbHandle);
    if (bResult == FALSE) {
        CloseHandle(hDeviceHandle);
        hDeviceHandle = INVALID_HANDLE_VALUE;
        return winusb_err_convert(GetLastError());
    }
    // TODO(Emma): set the timeouts

    return kDlDev_Success;
}

int dload_close_device(void)
{
    // make sure we have a device open
    if (hWinusbHandle == INVALID_HANDLE_VALUE)
        return kDlDev_DeviceNotOpen;

    // close it
    WinUsb_Free(hWinusbHandle);
    CloseHandle(hDeviceHandle);
    hWinusbHandle = INVALID_HANDLE_VALUE;
    hDeviceHandle = INVALID_HANDLE_VALUE;

    return kDlDev_Success;
}

int dload_device_send_packet(const uint8_t *buffer, size_t buffer_len)
{
    // make sure we have a device open
    if (hWinusbHandle == NULL)
        return kDlDev_DeviceNotOpen;

    // send the message
    ULONG ulTransferred = 0;
    BOOL bResult = WinUsb_WritePipe(hWinusbHandle, 0x01, (uint8_t *)buffer, buffer_len, &ulTransferred, NULL);

    if (bResult == FALSE) {
        return winusb_err_convert(GetLastError());
    }

    return kDlDev_Success;
}

int dload_device_recv_packet(uint8_t **out_buffer, size_t *out_len)
{
    // make sure we have a device open
    if (hWinusbHandle == NULL)
        return kDlDev_DeviceNotOpen;

    // allocate a buffer for our message
    void *full_msg = malloc(0x2020);
    if (full_msg == NULL)
        return kDlDev_OutOfMemory;

    *out_buffer = (uint8_t *)full_msg;
    ULONG ulTransferred = 0;
    // TODO(Emma): does the max output size (0x2020) change? will certain requests take longer + need a longer timeout?
    BOOL bResult = WinUsb_ReadPipe(hWinusbHandle, 0x81, (uint8_t *)full_msg, 0x2020, &ulTransferred, NULL);

    if (bResult)
        *out_len = ulTransferred & 0x7FFFFFFF;
    else
        return winusb_err_convert(GetLastError());

    return kDlDev_Success;
}

#endif // _WIN32
