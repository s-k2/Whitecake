#include "Hid.h"

#include <WinIoCtl.h>
#include <Setupapi.h>
#include <tchar.h>

typedef struct _HIDD_ATTRIBUTES {
  ULONG  Size;
  USHORT VendorID;
  USHORT ProductID;
  USHORT VersionNumber;
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

typedef USHORT USAGE;
typedef struct _HIDP_CAPS {
  USAGE  Usage;
  USAGE  UsagePage;
  USHORT InputReportByteLength;
  USHORT OutputReportByteLength;
  USHORT FeatureReportByteLength;
  USHORT Reserved[17];
  USHORT NumberLinkCollectionNodes;
  USHORT NumberInputButtonCaps;
  USHORT NumberInputValueCaps;
  USHORT NumberInputDataIndices;
  USHORT NumberOutputButtonCaps;
  USHORT NumberOutputValueCaps;
  USHORT NumberOutputDataIndices;
  USHORT NumberFeatureButtonCaps;
  USHORT NumberFeatureValueCaps;
  USHORT NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;

typedef void * PHIDP_PREPARSED_DATA;

typedef BOOLEAN (__stdcall *HidD_GetAttributesFunc) (
  /*_In_*/  HANDLE           HidDeviceObject,
  /*_Out_*/ PHIDD_ATTRIBUTES Attributes
);

typedef BOOLEAN (__stdcall *HidD_GetInputReportFunc) (
  /*_In_*/  HANDLE HidDeviceObject,
  /*_Out_*/ PVOID  ReportBuffer,
  /*_In_*/  ULONG  ReportBufferLength
);

typedef BOOLEAN (__stdcall *HidD_GetPreparsedDataFunc) (
  /*_In_*/  HANDLE               HidDeviceObject,
  /*_Out_*/ PHIDP_PREPARSED_DATA *PreparsedData
);

typedef long NTSTATUS;
typedef NTSTATUS (__stdcall *HidP_GetCapsFunc) (
  /*_In_*/  PHIDP_PREPARSED_DATA PreparsedData,
  /*_Out_*/ PHIDP_CAPS           Capabilities
);

typedef BOOLEAN (__stdcall *HidD_GetFeatureFunc) (
  /*_In_*/  HANDLE HidDeviceObject,
  /*_Out_*/ PVOID  ReportBuffer,
  /*_In_*/  ULONG  ReportBufferLength
);

typedef BOOLEAN (__stdcall *HidD_GetSerialNumberStringFunc) (
  /*_In_ */ HANDLE HidDeviceObject,
  /*_Out_*/ PVOID  Buffer,
  /*_In_ */ ULONG  BufferLength
);

HidD_GetAttributesFunc HidD_GetAttributes;
HidD_GetInputReportFunc HidD_GetInputReport;
HidD_GetPreparsedDataFunc HidD_GetPreparsedData;
HidP_GetCapsFunc HidP_GetCaps;
HidD_GetFeatureFunc HidD_GetFeature;
HidD_GetSerialNumberStringFunc HidD_GetSerialNumberString;

bool InitHidApi()
{
	HMODULE library = LoadLibrary(_T("Hid.dll"));
	if(library == NULL)
		return(false);

	HidD_GetAttributes = (HidD_GetAttributesFunc) GetProcAddress(library, "HidD_GetAttributes");
	HidD_GetInputReport = (HidD_GetInputReportFunc) GetProcAddress(library, "HidD_GetInputReport");
	HidD_GetPreparsedData = (HidD_GetPreparsedDataFunc) GetProcAddress(library, "HidD_GetPreparsedData");
	HidP_GetCaps = (HidP_GetCapsFunc) GetProcAddress(library, "HidP_GetCaps");
	HidD_GetFeature = (HidD_GetInputReportFunc) GetProcAddress(library, "HidD_GetFeature");
	HidD_GetSerialNumberString = (HidD_GetSerialNumberStringFunc) GetProcAddress(library, "HidD_GetSerialNumberString");

	return(true);
}

static volatile bool hidApiInitialized = InitHidApi();

static bool FindHid(TCHAR *path, size_t pathSize, short vendorId, short productId, int serial);
static bool IsReallyHIDClass(HDEVINFO devInfoSet, SP_DEVINFO_DATA *devInfoData);
static bool CheckIds(TCHAR *path, unsigned short vendor, unsigned short product, int serial);

bool WindowsHidOpen(HANDLE *hid, int vendorId, int productId, int serial)
{
	*hid = INVALID_HANDLE_VALUE;

	char hidPath[MAX_PATH];

	if(!FindHid(hidPath, sizeof(hidPath), vendorId, productId, serial))
		return(false);

	*hid = CreateFile(hidPath, 
		GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, 0, NULL);

	return(hid != INVALID_HANDLE_VALUE);
}

static bool FindHid(TCHAR *path, size_t pathSize, short vendorId, short productId, int serial)
{
	GUID InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };
	HDEVINFO devInfoSet;
	SP_DEVINFO_DATA devInfoData;
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	SP_DEVICE_INTERFACE_DETAIL_DATA *deviceInterfaceDetail = NULL;
	int deviceIndex = 0;

	memset(&devInfoData, 0x00, sizeof(devInfoData));
	devInfoData.cbSize = sizeof(devInfoData);
	deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

	devInfoSet = SetupDiGetClassDevs(&InterfaceClassGuid, 
		NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	for(;;) {
		if(!SetupDiEnumDeviceInterfaces(devInfoSet, NULL, 
			&InterfaceClassGuid, deviceIndex, &deviceInterfaceData))
		{
			break;
		}

		// get size
		DWORD requiredSize;
		if(!SetupDiGetDeviceInterfaceDetail(devInfoSet, 
			&deviceInterfaceData, NULL, 0, &requiredSize, NULL))
		{
			// Break???
		}

		deviceInterfaceDetail = (SP_DEVICE_INTERFACE_DETAIL_DATA *) malloc(requiredSize);
		deviceInterfaceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		if(!SetupDiGetDeviceInterfaceDetail(devInfoSet, 
			&deviceInterfaceData, deviceInterfaceDetail, requiredSize, NULL, NULL))
		{
			free(deviceInterfaceDetail);
			break;
		}

		if(IsReallyHIDClass(devInfoSet, &devInfoData)) {
			if(CheckIds(deviceInterfaceDetail->DevicePath, vendorId, productId, serial)) {
				if(_tcslen(deviceInterfaceDetail->DevicePath) + 1 > pathSize)
					return(false);

				_tcscpy(path, deviceInterfaceDetail->DevicePath);
				return(true);
			}
		}

		free(deviceInterfaceDetail);

		deviceIndex++;
	}

	return(false);
}

static bool IsReallyHIDClass(HDEVINFO devInfoSet, SP_DEVINFO_DATA *devInfoData)
{
	TCHAR driverName[256];
	
	DWORD i = 0;
	while(1) {
		if(!SetupDiEnumDeviceInfo(devInfoSet, i, devInfoData))
			break;

		if(!SetupDiGetDeviceRegistryProperty(devInfoSet, devInfoData, SPDRP_CLASS, 
			NULL, (BYTE *) driverName, sizeof(driverName), NULL))
		{
			return(false);
		}

		if(_tccmp(driverName, _T("HIDClass")) == 0) {
			if(SetupDiGetDeviceRegistryProperty(devInfoSet, devInfoData,
				SPDRP_DRIVER, NULL, (BYTE *) driverName, sizeof(driverName), NULL))
			{
				return(true);
			}
		}

		i++;
	}

	return(false);
}

static bool CheckIds(TCHAR *path, unsigned short vendor, unsigned short product, int serial)
{
	HANDLE hid = CreateFile(path, 0, 0, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hid == INVALID_HANDLE_VALUE)
		return(false);
	
	HIDD_ATTRIBUTES hidAttributes;
	hidAttributes.Size = sizeof(HIDD_ATTRIBUTES);
	HidD_GetAttributes(hid, &hidAttributes);

#if 0
	// TODO: Windows didn't return a valid serial number here... Why???
	if(serial != 0) {
		WCHAR serialStr[0x80];
		if(HidD_GetSerialNumberString(hid, serialStr, sizeof(serialStr)) == FALSE)
			return(false);

		serialStr[sizeof(serialStr - 1)] = 0;
		OutputDebugStringW(serialStr);
	}
#endif /* 0 */

	CloseHandle(hid);

	return(hidAttributes.VendorID == vendor && hidAttributes.ProductID == product);
}

bool WindowsHidClose(HANDLE hid)
{
	return(hid == INVALID_HANDLE_VALUE || CloseHandle(hid));
}

bool WindowsHidSendOutputReport(HANDLE hid, unsigned char *buffer, size_t size)
{
	DWORD writtenBytes = 0;
	return(WriteFile(hid, (LPVOID *) buffer, size, &writtenBytes, NULL) == TRUE);
}

bool WindowsHidGetFeatureReport(HANDLE hid, unsigned char *buffer, size_t size)
{
	return(HidD_GetFeature(hid, (LPVOID *) buffer, size) == TRUE);
}