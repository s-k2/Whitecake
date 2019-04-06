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

	static const int BaudRate; // Every Arduino Uno uses this baud
	static const unsigned char CommandGetSync;
	static const unsigned char CommandGetParameter;
	static const unsigned char CommandEnterProgMode;
	static const unsigned char CommandReadSign;
	static const unsigned char CommandLoadAddress;
	static const unsigned char CommandProgramPage;
	static const unsigned char CommandLeaveProgMode;

	static const unsigned char ResponseInSync;
	static const unsigned char ResponseOk;

	static const unsigned char ParameterSoftwareMajor;
	static const unsigned char ParameterSoftwareMinor;

	static const unsigned char SyncCrcEop;
};

#endif /* ARDUINO_UPLOADER_H */
