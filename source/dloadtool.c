#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <libusb.h>
#include "dload.h"
#include "device_connection.h"
#include "dload_constants.h"

void hexdump(void *buf, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        printf("%02x ", ((uint8_t *)buf)[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    // initialise the usb library
    init_usb();

    // detect device
    int r = dload_detect_device();
    if (r == kDlDev_Success)
        printf("successfully detected DLOAD device!\n");
    else if (r == kDlDev_NoneFound)
        printf("no devices detected\n");
    else if (r == kDlDev_DeviceAlreadyOpen)
        printf("a DLOAD device is already open\n");
    else if (r == kDlDev_MultipleFound)
        printf("several Nokia devices were detected\n");
    else if (r == kDlDev_QCModeFound)
        printf("a device in Qualcomm mode was found, this is only compatible with DLOAD bootloaders\n");
    else if (r == kDlDev_WPModeFound)
        printf("device in normal mode found, please put it into DLOAD mode\n");
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

    // terminate the usb library
    close_usb();
    return 0;
}
