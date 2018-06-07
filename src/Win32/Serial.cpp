#include "Serial.h"

#include <string>
#include <vector>
using namespace std;

#include <initguid.h>
#include <devguid.h>
#include <setupapi.h>

static vector<pair<string, string>> NativeSerialEnumerate()
{
	vector<pair<string, string>> result;

	SP_DEVINFO_DATA devInfoData;
	devInfoData.cbSize = sizeof(devInfoData);

	// find all serial ports
	HDEVINFO hDeviceInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, 0, NULL, DIGCF_PRESENT);
	if(hDeviceInfo == INVALID_HANDLE_VALUE)
		return(result);

	for(int deviceNum = 0; SetupDiEnumDeviceInfo(hDeviceInfo, deviceNum, &devInfoData); deviceNum++) {
		char friendlyNameBuffer[1024] = { 0x00 };
		string friendlyName;
		if(!SetupDiGetDeviceRegistryProperty(hDeviceInfo, &devInfoData, SPDRP_FRIENDLYNAME, NULL, (LPBYTE) friendlyNameBuffer, sizeof(friendlyNameBuffer), NULL))
			friendlyName = "(unknown port)";
		else
			friendlyName = friendlyNameBuffer;

		HKEY registryKey = SetupDiOpenDevRegKey(hDeviceInfo, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
		if(registryKey != INVALID_HANDLE_VALUE) {
			char serialPortName[1024] = { 0x00 };
			DWORD serialPortNameSize = sizeof(serialPortName);
			if(RegQueryValueEx(registryKey, "PortName", NULL, NULL, (LPBYTE) serialPortName, &serialPortNameSize) == ERROR_SUCCESS) {
				result.push_back(make_pair(friendlyName, "\\\\.\\" + string(serialPortName)));
			}

			RegCloseKey(registryKey);
		}
	}

	return(result);
}


bool NativeSerialOpen(NativeSerial *port, string path, int baud, int dataBits, Parity parity, StopBits stopBits)
{
	if(path == "COMx") {
		auto availPorts = NativeSerialEnumerate();
		if(availPorts.size() == 1)
			path = availPorts.front().second;
	}

	*port = INVALID_HANDLE_VALUE;

	if(path.size() < 8 || path.substr(8) != "\\\\.\\COM")
		path = "\\\\.\\" + path;

	*port = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

	if(*port == INVALID_HANDLE_VALUE)
		return(false);

	DCB dcb;
	memset(&dcb, 0, sizeof(dcb));
	dcb.DCBlength = sizeof(DCB);
	
	if(!GetCommState(*port, &dcb)) {
		CloseHandle(port);
		*port = INVALID_HANDLE_VALUE;

		return(false);
	}

	dcb.BaudRate = baud;
	dcb.ByteSize = (BYTE) dataBits;
	if(parity == ParityNone)
		dcb.Parity = NOPARITY;
	else if(parity == ParityEven)
		dcb.Parity = EVENPARITY;
	else if(parity == ParityOdd)
		dcb.Parity = ODDPARITY;

	if(stopBits == StopBitsOne)
		dcb.StopBits = ONESTOPBIT;
	else if(stopBits == StopBitsOnePointFive)
		dcb.StopBits = ONE5STOPBITS;
	else if(stopBits == StopBitsTwo)
		dcb.StopBits = TWOSTOPBITS;

	if(!SetCommState(*port, &dcb)) {
		CloseHandle(port);
		*port = INVALID_HANDLE_VALUE;

		return(false);
	}
	
	return(true);
}

void NativeSerialClose(NativeSerial *port)
{
	CloseHandle(*port);
	*port = INVALID_HANDLE_VALUE;
}

int NativeSerialGetAvailableBytes(NativeSerial port)
{
	COMSTAT comstat;
	
	if(ClearCommError(port, NULL, &comstat) == FALSE)
		return(0); // we do not show a real errors... we just say, no byte is available

	return(comstat.cbInQue);
}

bool NativeSerialReceiveByte(NativeSerial port, char &byte)
{
	DWORD bytesRead;
	return(ReadFile(port, &byte, 1, &bytesRead, NULL) == TRUE);
}

int NativeSerialReceiveBytes(NativeSerial port, void *buffer, size_t maxNumber)
{
	DWORD bytesRead;
	if(ReadFile(port, buffer, maxNumber, &bytesRead, NULL) == FALSE)
		return(-1);
	else
		return(bytesRead);
}

bool NativeSerialSend(NativeSerial port, char byte)
{
	DWORD writtenBytes;
	// will even return false if port is not open
	return(WriteFile(port, &byte, 1, &writtenBytes, NULL) == TRUE); 
}

bool NativeSerialSend(NativeSerial port, void *buffer, size_t count)
{
	DWORD writtenBytes;
	return(WriteFile(port, buffer, count, &writtenBytes, NULL) == TRUE);
}
