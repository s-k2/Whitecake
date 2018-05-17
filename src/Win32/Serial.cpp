#include "Serial.h"

#include <string>
using namespace std;

bool NativeSerialOpen(NativeSerial *port, string path, int baud, int dataBits, Parity parity, StopBits stopBits)
{
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
