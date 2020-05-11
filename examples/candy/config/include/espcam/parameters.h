/* This file is gerated by parameters-to-c.py script, do not modify. */
OSAL_C_HEADER_BEGINS


typedef struct candy_persistent
  {
  os_char size[10];
  os_uchar quality;
  os_uchar brightness;
  os_uchar contrast;
  os_uchar hue;
  os_uchar saturation;
}
candy_persistent;

typedef struct candy_volatile
  {
  os_boolean on;
}
candy_volatile;

#ifndef IOBOARD_DEVICE_NAME
#define IOBOARD_DEVICE_NAME "candy"
#endif

OSAL_C_HEADER_ENDS
