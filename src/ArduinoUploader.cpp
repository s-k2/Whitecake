#include "ArduinoUploader.h"

#include "Translate.h"

using std::vector;

const int ArduinoUploader::BaudRate = 115200; // Every Arduino Uno uses this baud
const unsigned char ArduinoUploader::CommandGetSync = 0x30;
const unsigned char ArduinoUploader::CommandGetParameter = 0x41;
const unsigned char ArduinoUploader::CommandEnterProgMode = 0x50;
const unsigned char ArduinoUploader::CommandReadSign = 0x75;
const unsigned char ArduinoUploader::CommandLoadAddress = 0x55;
const unsigned char ArduinoUploader::CommandProgramPage = 0x64;
const unsigned char ArduinoUploader::CommandLeaveProgMode = 0x51;
const unsigned char ArduinoUploader::ResponseInSync = 0x14;
const unsigned char ArduinoUploader::ResponseOk = 0x10;
const unsigned char ArduinoUploader::ParameterSoftwareMajor = 0x81;
const unsigned char ArduinoUploader::ParameterSoftwareMinor = 0x82;
const unsigned char ArduinoUploader::SyncCrcEop = 0x20;

ArduinoUploader::ArduinoUploader(std::string portPath, const std::vector<unsigned char> &program, int *threadStop, int *threadState)
	: port(portPath, BaudRate), program(program), threadStop(threadStop), threadState(threadState)
{
	if(!port.IsOpen())
		throw UploaderException(TR_COULD_NOT_OPEN_COMPORT);

	ResetBoard();
	SendInitialSyncs();
	GetVersion();
	EnterProgMode();
	ReadSignature();
	SendProgram();
	LeaveProgMode();
	ResetBoard();
}

void ArduinoUploader::ResetBoard()
{
	port.ClearDtrRts(); // clear to unload reset capacitor
	NativeSleep(50);
	port.SetDtrRts();
	Sleep(250);
}

void ArduinoUploader::SendInitialSyncs()
{
	for(size_t tries = 0, success = 0; success < 4; tries++) {
		if(StkGetSync())
			break;//success++;
		if(tries > 8)
			throw UploaderException(std::string(TR_CHIP_DID_NOT_RESPONDDIDNT_YOU_RESETIS_IT_TURNED_OFFIS_IT_CONNECTED) + "\n (no sync)");
	}
}

bool ArduinoUploader::StkGetSync()
{
	ClearRubbish();

	StkSendCommand(CommandGetSync);

	auto response = StkReceiveResponse(500);
	return(response.size() == 1 && response.front() == ResponseInSync);
}

void ArduinoUploader::GetVersion()
{
	StkSendCommand(CommandGetParameter, ParameterSoftwareMajor);
	auto response = StkReceiveResponse();
	if(response.size() != 2 || response[0] != ResponseInSync)
		throw UploaderException(std::string(TR_CHIP_DID_NOT_RESPONDDIDNT_YOU_RESETIS_IT_TURNED_OFFIS_IT_CONNECTED) + "\n (invalid version)");

	StkSendCommand(CommandGetParameter, ParameterSoftwareMinor);
	response = StkReceiveResponse();
	if(response.size() != 2 || response[0] != ResponseInSync)
		throw UploaderException(std::string(TR_CHIP_DID_NOT_RESPONDDIDNT_YOU_RESETIS_IT_TURNED_OFFIS_IT_CONNECTED) + "\n (invalid version)");
}

void ArduinoUploader::EnterProgMode()
{
	StkSendCommand(CommandEnterProgMode);
	StkExpectResponse(ResponseInSync);
}

void ArduinoUploader::ReadSignature()
{
	StkSendCommand(CommandReadSign);
	auto response = StkReceiveResponse();
	if(response.size() != 4 || response[0] != ResponseInSync ||
		response[1] != 0x1e || response[2] != 0x95 || response[3] != 0x0f)
	{
		throw UploaderException(std::string(TR_CHIP_DID_NOT_RESPONDDIDNT_YOU_RESETIS_IT_TURNED_OFFIS_IT_CONNECTED) + "\n (invalid avr-signature)");
	}
}

void ArduinoUploader::SendProgram()
{
	static const size_t PageSize = 0x80; // TODO: Should we accept other Atmels too?

	FillProgramPages(PageSize);
	for(size_t i = 0; i < program.size(); i += PageSize) {
		OutputDebugString("Sending a page...\n");
		LoadAddress(i / 2);
		SendPage(&program[i], PageSize);
	}
}

void ArduinoUploader::FillProgramPages(size_t pageSize)
{
	while(program.size() % pageSize)
		program.push_back(0xff);
}

void ArduinoUploader::LoadAddress(size_t address)
{
	vector<unsigned char> loadAddressCmd;
	loadAddressCmd.push_back(CommandLoadAddress);
	loadAddressCmd.push_back(address & 0xff); // lsb
	loadAddressCmd.push_back((address >> 8) & 0xff); // msb
	StkSendCommand(loadAddressCmd);
	StkExpectResponse(ResponseInSync);
}

void ArduinoUploader::SendPage(unsigned char *bytes, size_t pageSize)
{
	vector<unsigned char> progPageCmd;
	progPageCmd.push_back(CommandProgramPage);
	progPageCmd.push_back((pageSize >> 8) & 0xff); // page-size (msb)
	progPageCmd.push_back(pageSize & 0xff); // page-size (lsb)
	progPageCmd.push_back('F'); // program to 'F'lash
	for(size_t i = 0; i < pageSize; i++) {
		progPageCmd.push_back(bytes[i]);
	}
	StkSendCommand(progPageCmd);
	StkExpectResponse(ResponseInSync);
}

void ArduinoUploader::LeaveProgMode()
{
	StkSendCommand(CommandLeaveProgMode);
	StkExpectResponse(ResponseInSync);
}

void ArduinoUploader::ClearRubbish()
{
	char rubbish;

	while(port.CanRead())
		port.ReceiveByte(rubbish);
}

void ArduinoUploader::StkSendCommand(unsigned char byte)
{
	vector<unsigned char> command;
	command.push_back(byte);

	StkSendCommand(command);
}

void ArduinoUploader::StkSendCommand(unsigned char byte0, unsigned char byte1)
{
	vector<unsigned char> command;
	command.push_back(byte0);
	command.push_back(byte1);

	StkSendCommand(command);
}

void ArduinoUploader::StkSendCommand(std::vector<unsigned char> &bytes)
{
	bytes.push_back(SyncCrcEop);
	port.Send(&bytes[0], bytes.size());
}

std::vector<unsigned char> ArduinoUploader::StkReceiveResponse(size_t timeout)
{
	std::vector<unsigned char> response;

	size_t elapsedTime = 0;
	do {
		while(!port.CanRead()) {
			NativeSleep(10);
			elapsedTime += 10;
			if(elapsedTime > timeout)
				return(std::vector<unsigned char>());
		}

		char readByte;
		port.ReceiveByte(readByte);
		response.push_back((unsigned char) readByte);
	} while(response.back() != ResponseOk);

	response.pop_back(); // last character is not needed
	return(response);
}

void ArduinoUploader::StkExpectResponse(char byte)
{
	auto response = StkReceiveResponse();
	if(response.size() != 1 || response[0] != byte)
			throw UploaderException(std::string(TR_CHIP_DID_NOT_RESPONDDIDNT_YOU_RESETIS_IT_TURNED_OFFIS_IT_CONNECTED) + "\n (unexpected/missing byte response)");
}

void ArduinoUploader::StkExpectResponse(char byte0, char byte1)
{
	auto response = StkReceiveResponse();
	if(response.size() != 2 || 
		response[0] != byte0 ||
		response[1] != byte1)
	{
			throw UploaderException(std::string(TR_CHIP_DID_NOT_RESPONDDIDNT_YOU_RESETIS_IT_TURNED_OFFIS_IT_CONNECTED) + "\n (unexpected/missing two-byte response");
	}
}
