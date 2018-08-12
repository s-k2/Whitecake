#ifndef ARDUINO_UPLOADER_H
#define ARDUINO_UPLOADER_H

#include "Uploader.h"

class ArduinoUploader : public Uploader
{
public:
	ArduinoUploader(std::string port, const std::vector<unsigned char> &program, int *threadStop, int *threadState);

private:
	void ResetBoard();
	void SendInitialSyncs();
	void GetVersion();
	void EnterProgMode();
	void ReadSignature();
	void SendProgram();
	void FillProgramPages(size_t pageSize);
	void LoadAddress(size_t address);
	void SendPage(unsigned char *bytes, size_t pageSize);
	void LeaveProgMode();

	bool StkGetSync();

	void ClearRubbish();
	void StkSendCommand(unsigned char byte);
	void StkSendCommand(unsigned char byte0, unsigned char byte1);
	void StkSendCommand(std::vector<unsigned char> &bytes);
	std::vector<unsigned char> StkReceiveResponse(size_t timeout = 100);
	void StkExpectResponse(char byte);
	void StkExpectResponse(char byte0, char byte1);

	Serial port;
	std::vector<unsigned char> program;

	int *threadStop; // get info, we should stop
	int *threadState; // show our progress

	static const int BaudRate = 115200; // Every Arduino Uno uses this baud
	static const unsigned char CommandGetSync = 0x30;
	static const unsigned char CommandGetParameter = 0x41;
	static const unsigned char CommandEnterProgMode = 0x50;
	static const unsigned char CommandReadSign = 0x75;
	static const unsigned char CommandLoadAddress = 0x55;
	static const unsigned char CommandProgramPage = 0x64;
	static const unsigned char CommandLeaveProgMode = 0x51;

	static const unsigned char ResponseInSync = 0x14;
	static const unsigned char ResponseOk = 0x10;

	static const unsigned char ParameterSoftwareMajor = 0x81;
	static const unsigned char ParameterSoftwareMinor = 0x82;

	static const unsigned char SyncCrcEop = 0x20;
};

#endif /* ARDUINO_UPLOADER_H */
