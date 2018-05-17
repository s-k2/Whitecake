#ifndef WIN32_SERIAL_H
#define WIN32_SERIAL_H

#include <string>
#include <windows.h>

typedef HANDLE NativeSerial;
enum Parity { ParityNone, ParityEven, ParityOdd };
enum StopBits { StopBitsOne, StopBitsOnePointFive, StopBitsTwo };

bool NativeSerialOpen(NativeSerial *port, std::string path, 
	int baud = 9600, int dataBits = 8, Parity parity = ParityNone, 
	StopBits stopBits = StopBitsOne);
void NativeSerialClose(NativeSerial *port);

int NativeSerialGetAvailableBytes(NativeSerial port);

bool NativeSerialReceiveByte(NativeSerial port, char &byte);
int NativeSerialReceiveBytes(NativeSerial port, void *buffer, size_t maxNumber);

bool NativeSerialSend(NativeSerial port, char byte);
bool NativeSerialSend(NativeSerial port, void *buffer, size_t count);

inline bool NativeSerialIsOpen(NativeSerial port)
	{ return(port != INVALID_HANDLE_VALUE); };

inline bool NativeSerialSetRTSDTR(NativeSerial port)
	{ 	return(EscapeCommFunction(port, SETRTS) != 0 &&
			EscapeCommFunction(port, SETDTR) != 0); };

inline bool NativeSerialClearRTSDTR(NativeSerial port)
	{ 	return(EscapeCommFunction(port, CLRRTS) != 0 &&
			EscapeCommFunction(port, CLRDTR) != 0); };

#endif /* WIN32_SERIAL_H */