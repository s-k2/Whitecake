#include "HidPort.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include "Win32/Hid.h"

typedef HANDLE pthread_t;
#define pthread_attr_default 0
#define pthread_create(thread, threadAttribute, threadFunction, arg) \
	(*thread = CreateThread(NULL, (threadAttribute), (LPTHREAD_START_ROUTINE) threadFunction, arg, 0, (LPDWORD) NULL))
inline void pthread_join(pthread_t thread, void **retValue)
{
	WaitForSingleObject(thread, INFINITE);
	if(retValue != NULL)
		GetExitCodeThread(thread, (DWORD *) retValue);
}

typedef HANDLE pthread_mutex_t;
#define pthread_mutexattr_default MUTEX_ALL_ACCESS
#define pthread_mutex_init(mutex, attr) \
	*(mutex) = CreateMutex(NULL, FALSE, NULL);
#define pthread_mutex_lock(mutex) \
	WaitForSingleObject(*(mutex), INFINITE);
#define pthread_mutex_unlock(mutex) \
	ReleaseMutex(*(mutex))
#define pthread_mutex_destroy(mutex) \
	CloseHandle(*(mutex));


#else
#include "Gtk/Hid.h"
#include <pthread.h>

#endif /* _WIN32 */

static const size_t InvalidBufferIndex = -1;

/* 
 * Set this value depending on your application
 * If you won't check for new data constantly maybe you should raise this
 * value? This still needs some more testing
 * By the way.. it should be a power of 2!
 */
static const size_t InputBufferSize = 0x800;
static const size_t InputBufferWrapMask = InputBufferSize - 1;

/*
 * Depending on your application you should adjust output-buffer's size
 * In my tests it was usefull to set this size to 0x2000 for echo-tests
 * But if you're sending much less than you'll receive a lower value
 * would save some memory...
 * By the way.. it should be a power of 2!
 */
static const size_t OutputBufferSize = 0x2000;
static const size_t OutputBufferWrapMask = OutputBufferSize - 1;

static const size_t HidReportSize = 9;

struct HidPort
{
	pthread_mutex_t inputBufferMutex;
	unsigned char inputBuffer[InputBufferSize];
	size_t inputBufferWriter;
	size_t inputBufferReader;
	pthread_mutex_t outputBufferMutex;
	unsigned char outputBuffer[OutputBufferSize];
	size_t outputBufferWriter;
	size_t outputBufferReader;

	/*
	 * The following two variables are assumed to be accessed atomic!
	 * If you're porting this to platforms other x86/amd64 assure that...
	 */
	size_t writeDeviceFree; // how many bytes could be written to the device?

	size_t errorState;
	enum ErrorState { Running = 0, Stop, ReadError, WriteError };
	inline bool HasErrorOccured()
		{ return(errorState != Running); };
	inline void SetErrorState(enum ErrorState newErrorState)
		{ errorState = newErrorState; };

	NativeHid hid;

	pthread_t workerThread;
};

static void *HidPortWorkerThreadMain(void *hidPortVoid);

int HidPortOpen(struct HidPort **portPtr, int vendorId, int productId, int serial)
{
	struct HidPort *port = new HidPort;
	*portPtr = port;

	pthread_mutex_init(&port->inputBufferMutex, NULL);
	pthread_mutex_init(&port->outputBufferMutex, NULL);
	port->inputBufferWriter = 0;
	port->inputBufferReader = InvalidBufferIndex;
	port->outputBufferWriter = 0;
	port->outputBufferReader = InvalidBufferIndex;
	port->writeDeviceFree = 0;

	port->errorState = 0;

	if(!NativeHidOpen(&port->hid, vendorId, productId, serial)) {
		delete port;
		*portPtr = NULL;
		return(-1);
	}

	// now create the worker thread that constantly receives and sends data
	pthread_create(&port->workerThread, pthread_attr_default, &HidPortWorkerThreadMain, port);

	return(0);
}

void HidPortClose(struct HidPort *port)
{
	if(port == NULL) // nothing to to here...
		return;

	port->errorState = HidPort::Stop;

	pthread_join(port->workerThread, NULL);

	pthread_mutex_destroy(&port->inputBufferMutex);
	pthread_mutex_destroy(&port->outputBufferMutex);

	NativeHidClose(port->hid);
}


int HidPortReceiveBytes(struct HidPort *port, unsigned char *data, size_t size)
{
	if(port->errorState > 0)
		return(-1);

	pthread_mutex_lock(&port->inputBufferMutex);

	size_t readSize = 0;
	for(; size > 0 && port->inputBufferReader != port->inputBufferWriter && 
			port->inputBufferReader != InvalidBufferIndex; data++, size--, readSize++) 
	{
		*data = port->inputBuffer[port->inputBufferReader];
		port->inputBufferReader = (port->inputBufferReader + 1) & InputBufferWrapMask;
	}

	if(port->inputBufferReader == port->inputBufferWriter)
		port->inputBufferReader = InvalidBufferIndex;

	pthread_mutex_unlock(&port->inputBufferMutex);

	//Sleep(10);

	return(readSize);
}

int HidPortSendBytes(struct HidPort *port, unsigned char *data, size_t size)
{
	if(port->errorState > 0)
		return(-1);

	int ret = 0;

	pthread_mutex_lock(&port->outputBufferMutex);

	if(size > 0 && port->outputBufferReader == InvalidBufferIndex)
		port->outputBufferReader = port->outputBufferWriter;

	for(; size > 0; data++, size--) {
		port->outputBuffer[port->outputBufferWriter] = *data;
		port->outputBufferWriter = (port->outputBufferWriter + 1) & OutputBufferWrapMask;
		
		if(port->outputBufferReader == port->outputBufferWriter) {
			ret = -1;
			break;
		}
	}

	pthread_mutex_unlock(&port->outputBufferMutex);

	return(ret);
}


int HidPortCanRead(struct HidPort *port)
{
	if(port->errorState > 0)
		return(-1);

	pthread_mutex_lock(&port->inputBufferMutex);

	int canRead = (port->inputBufferReader != InvalidBufferIndex);

	pthread_mutex_unlock(&port->inputBufferMutex);

	return(canRead);
}

void HidPortAppendToInputBuffer(struct HidPort *port, unsigned char *data, size_t size)
{
	pthread_mutex_lock(&port->inputBufferMutex);

	if(size > 0 && port->inputBufferReader == InvalidBufferIndex)
		port->inputBufferReader = port->inputBufferWriter;

	for(; size > 0; data++, size--) {
		port->inputBuffer[port->inputBufferWriter] = *data;
		port->inputBufferWriter = (port->inputBufferWriter + 1) & InputBufferWrapMask;
		
		if(port->inputBufferReader == port->inputBufferWriter) {
			//fprintf(stderr, "inputBuffer full!\n");
			// TODO: Currently we silently ignore bytes if the input-buffer is full
			break;
		}

	}

	pthread_mutex_unlock(&port->inputBufferMutex);
}

bool HidPortReadBlock(struct HidPort *port)
{
	unsigned char buffer[HidReportSize];
	buffer[0] = 0x00;

	if(!NativeHidGetFeatureReport(port->hid, buffer, sizeof(buffer))) {
		port->errorState = HidPort::ReadError;
		port->writeDeviceFree = 0;
		return(false);
	}

	port->writeDeviceFree = buffer[1] >> 3;

    size_t packetSize = buffer[1] & 7;

	if(packetSize > 0)
		HidPortAppendToInputBuffer(port, &buffer[2], packetSize);

	return(true);
}

#define HARDWARE_INPUT_BUFFER_SIZE 7

void HidPortWriteIfPossible(struct HidPort *port)
{
	pthread_mutex_lock(&port->outputBufferMutex);

	// nothing to send to the device, so just exit
	if(port->outputBufferReader == InvalidBufferIndex) {
		pthread_mutex_unlock(&port->outputBufferMutex);
		return;
	}

	// how many bytes could be send? 
	// Be aware of the wrap-around with this calculation
	size_t outputBufferSize;
	if(port->outputBufferReader < port->outputBufferWriter)
		outputBufferSize = port->outputBufferWriter - port->outputBufferReader;
	else
		outputBufferSize = OutputBufferSize + port->outputBufferWriter - port->outputBufferReader;

	if(outputBufferSize > 0 && // if there is something to send...
		port->writeDeviceFree > 0 &&  // and there is space...
		// and it makes sense to send now (and not to  wait longer)...
		// ...because either the content of output-buffer is small enough
		((outputBufferSize < HARDWARE_INPUT_BUFFER_SIZE && outputBufferSize <= port->writeDeviceFree) 
			|| port->writeDeviceFree >= HARDWARE_INPUT_BUFFER_SIZE)) // ...or we can send a full packet
	{
		unsigned char buffer[HidReportSize];
		size_t packetSize = outputBufferSize < HARDWARE_INPUT_BUFFER_SIZE ? outputBufferSize : HARDWARE_INPUT_BUFFER_SIZE;

		if(port->writeDeviceFree < packetSize)
			MessageBoxA(NULL, "ERROR", "ERROR", 32);
		buffer[0] = 0x00;
		buffer[1] = packetSize & 0x7;
		for(size_t i = 0; i < packetSize; i++) {
			buffer[2 + i] = port->outputBuffer[port->outputBufferReader];
			port->outputBufferReader = (port->outputBufferReader + 1) & OutputBufferWrapMask;
		}
		//port->writeDeviceFree = 0; // just to be sure how much is free for the next send

		if(port->outputBufferReader == port->outputBufferWriter)
			port->outputBufferReader = InvalidBufferIndex;

		pthread_mutex_unlock(&port->outputBufferMutex);
		if(!NativeHidSendOutputReport(port->hid, buffer, sizeof(buffer))) {
			port->errorState = HidPort::WriteError;
		}
	} else {
		pthread_mutex_unlock(&port->outputBufferMutex);
	}
}

void *HidPortWorkerThreadMain(void *hidPortVoid)
{
	struct HidPort *port = (struct HidPort *) hidPortVoid;

	while(!port->errorState)
		if(HidPortReadBlock(port))
			HidPortWriteIfPossible(port);

	return(NULL);
}
