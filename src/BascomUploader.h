#ifndef BASCOM_UPLOADER_H
#define BASCOM_UPLOADER_H

#include "Uploader.h"

class BascomUploader : public Uploader
{
public:
	BascomUploader(std::string port, int baud, const std::vector<unsigned char> &program, int *threadStop, int *threadState);

private:
	void SendWithHid();
	void SendWithSerial();
	void SendWorker(Port *port);
	
	bool FirstHello(Port *port);
	bool SecondHello(Port *port);
	bool SendBinary(Port *port);
	bool SendBlock(Port *port, unsigned char *bytes, size_t length);
	bool WaitForByte(Port *port, char byte);
	bool ReadNextByte(Port *port, char &byte);
	
	static const int Timeout;
	static const int SleepTime;
	static const int MaxTries;
	static const int BlockMaxRetries;
	static const unsigned char PaddingChar;

	std::string port;
	int baud;
	const std::vector<unsigned char> &program;

	int *threadStop; // get info, we should stop
	int *threadState; // show our progress
};

#endif /* BASCOM_UPLOADER_H */