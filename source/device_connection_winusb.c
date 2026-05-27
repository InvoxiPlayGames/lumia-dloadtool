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

int dload_init_usb()
{
    // WinUSB doesn't need this
    return kDlDev_Success;
}

void dload_close_usb()
{
    // WinUSB doesn't need this
}

int dload_detect_device()
{
    // if we already have a device, report back and don't scan again
    if (hDeviceHandle != INVALID_HANDLE_VALUE)
        return kDlDev_DeviceAlreadyOpen;
    
    int r = kDlDev_NoneFound;

    // TODO(Emma): detect devices present in the other modes (main, FFU/SLDR, BLDR, mass storage)
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
            wr = CM_MapCrToWin32Err(cr, ERROR_INVALID_DATA);
            break;
        }

        DeviceInterfaceList = (PSTR)malloc(DeviceInterfaceListLength);

        if (DeviceInterfaceList == NULL) {
            return kDlDev_OutOfMemory;
        }

        cr = CM_Get_Device_Interface_ListA((LPGUID)&GUID_DEVINTERFACE_CareSuiteUSB, NULL, DeviceInterfaceList, DeviceInterfaceListLength, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

        if (cr != CR_SUCCESS) {
            free(DeviceInterfaceList);
            if (cr != CR_BUFFER_SMALL) {
                wr = CM_MapCrToWin32Err(cr, ERROR_INVALID_DATA);
            }
        }
    } while (cr == CR_BUFFER_SMALL);

    // TODO(Emma): find out what error codes are meaningful from configmgr
    if (wr != 0) {
        printf("ConfigMgr Error: 0x%08x, Win: 0x%08x\n", cr, wr);
        r = winusb_err_convert(wr);
        goto end;
    }

    if (DeviceInterfaceList[0] != 0) {
        // detect if we have several devices
        if (DeviceInterfaceListLength > strlen(DeviceInterfaceList) + 1)
            r = kDlDev_MultipleFound;
        printf("Found WinUsb device: %s\n", DeviceInterfaceList);
        // TODO(Emma): check VID/PID to see if it's actually DLOAD and not another mode

        hDeviceHandle = CreateFileA(DeviceInterfaceList, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
        if (hDeviceHandle != NULL) {
            r = kDlDev_Success;
        } else {
            r = winusb_err_convert(GetLastError());
        }
    }

end:
    return r;
}

int dload_open_device()
{
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
        return kDlDev_NoneFound;

    BOOL bResult = WinUsb_Initialize(hDeviceHandle, &hWinusbHandle);
    if (bResult == FALSE) {
        CloseHandle(hDeviceHandle);
        hDeviceHandle = INVALID_HANDLE_VALUE;
        return winusb_err_convert(GetLastError());
    }

    return kDlDev_Success;
}

int dload_close_device()
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
