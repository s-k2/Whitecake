#ifndef WINDOWS_HID_DEVICE_H
#define WINDOWS_HID_DEVICE_H

#include <string>
#include <windows.h>
typedef HANDLE WindowsHid;

bool WindowsHidOpen(HANDLE *hid, int vendorId, int productId, int serial);
bool WindowsHidClose(HANDLE hid);
bool WindowsHidSendOutputReport(HANDLE hid, unsigned char *buffer, size_t size);
bool WindowsHidGetFeatureReport(HANDLE hid, unsigned char *buffer, size_t size);

typedef WindowsHid NativeHid;
#define NativeHidOpen WindowsHidOpen
#define NativeHidClose WindowsHidClose
#define NativeHidSendOutputReport WindowsHidSendOutputReport
#define NativeHidGetFeatureReport WindowsHidGetFeatureReport

#endif /* WINDOWS_HID_DEVICE_H */
