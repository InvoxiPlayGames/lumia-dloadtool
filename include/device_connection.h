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
    kDlDev_WinUsb_ConfigErr = -3098,
    kDlDev_WinUsb_Unknown = -3099,
    // misc random errors
    kDlDev_OutOfMemory = -4444,
} dload_device_errors_t;

int dload_init_usb();
void dload_close_usb();

int dload_detect_device();
int dload_open_device();
int dload_close_device();

int dload_device_send_packet(const uint8_t *buffer, size_t buffer_len);
int dload_device_recv_packet(uint8_t **out_buffer, size_t *out_len);

#endif // DEVICE_CONNECTION_H
