#ifndef HID_PORT_H
#define HID_PORT_H

#include <stddef.h>

struct HidPort;

extern int HidPortOpen(struct HidPort **portPtr, int vendorId, int productId, int serial);
extern void HidPortClose(struct HidPort *port);
extern int HidPortReceiveBytes(struct HidPort *port, unsigned char *data, size_t size);
extern int HidPortSendBytes(struct HidPort *port, unsigned char *data, size_t size);
extern int HidPortCanRead(struct HidPort *port);

#endif /* HID_PORT_H */
