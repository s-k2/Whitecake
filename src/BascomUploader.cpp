#include "BascomUploader.h"

using std::string;
using std::vector;

#include "Platform.h"
#include "Translate.h"

const int BascomUploader::Timeout = 6000;
const int BascomUploader::SleepTime = 100;
const int BascomUploader::MaxTries = BascomUploader::Timeout / BascomUploader::SleepTime;
const int BascomUploader::BlockMaxRetries = 5;
const unsigned char BascomUploader::PaddingChar = 0x1A; // is this ok???

BascomUploader::BascomUploader(std::string port, const std::vector<unsigned char> &program, int *threadStop, int *threadState)
	: port(port), program(program), threadStop(threadStop), threadState(threadState)
{
	if(port == "HID")
		SendWithHid();
	else
		SendWithSerial();
}

void BascomUploader::SendWithHid()
{
	Hid hid(0x16C0, 0x05DC);
	SendWorker(&hid);
}

void BascomUploader::SendWithSerial()
{
	Serial serial(port, SerialBaud);
	SendWorker(&serial);
}

void BascomUploader::SendWorker(Port *port)
{
	(*threadState)++;
	if(!port->IsOpen())
		throw UploaderException(TR_COULD_NOT_OPEN_COMPORT);
	
	if(!FirstHello(port))
		throw UploaderException(TR_CHIP_DID_NOT_RESPONDDIDNT_YOU_RESETIS_IT_TURNED_OFFIS_IT_CONNECTED);
	
	(*threadState)++;
	if(!SecondHello(port))
		throw UploaderException(TR_CHIP_DID_NOT_RESPOND_ON_SECOND_HELLOIS_A_PROGRAMM_RUNNING_THAT_SENDS_);

	if(!WaitForByte(port, 0x15))
		throw UploaderException(TR_CHIP_DID_NOT_SEND_0X15IS_A_PROGRAMM_RUNNING_THAT_SENDS_);

	if(!SendBinary(port))
		throw UploaderException(TR_COULD_NOT_SEND_PROGRAM);
	
	port->Send(0x04);
	if(!WaitForByte(port, 0x06))
		throw UploaderException(TR_DID_NOT_RECEIVE_LAST_0X06);
}

bool BascomUploader::FirstHello(Port *port)
{
	// read out all rubbish in port's buffer
	while(port->CanRead()) {
		char response;
		port->ReceiveByte(response); // ignore that character
	}
	
	// wait forever for an response of the bootloader
	while(*threadStop == 0) {
		port->Send('{');

		// maybe there is some other rubbish in the queue of the serial port...
		// because of this we use a loop to read all available bytes here and 
		// ignore all rubbish in that loop but do not send '{' again, because the 
		// chip could understand this as the second hello (and we would get 
		// problems in the second hello...)
		while(port->CanRead()) {
			char response;

			if(port->ReceiveByte(response) && response == '{')
				return(true);
		}

		NativeSleep(SleepTime);
	}

	return(false);
}

bool BascomUploader::SecondHello(Port *port)
{
	port->Send('{');
	return(WaitForByte(port, '{'));
}

bool BascomUploader::SendBinary(Port *port)
{
	static const int PacketLen = 132; // XMODEM packet length
	static const int PacketHeaderLen = 3; // header of XMODEM has three bytes
	static const int BlockLen = 128; // XMODEM packet contains 128 data bytes

	size_t pos = 0;
	unsigned char packet[PacketLen];
	packet[0] = 0x01;
	packet[1] = 0x01;
	packet[2] = 0xFE;

	while(pos < program.size()) {
		// if there are not 128 bytes available, copy the rest and add fill bytes
		if(pos + BlockLen > program.size()) {
			memcpy(packet + PacketHeaderLen, &program[0] + pos, program.size() - pos);
			for(size_t i = program.size() - pos + PacketHeaderLen; i < PacketLen - 1; i++)
				packet[i] = PaddingChar;

			pos = program.size();
		} else { // if enough are available copy next 128
			memcpy(packet + PacketHeaderLen, &program[0] + pos, BlockLen);
			pos += BlockLen;
		}

		// calculate the checksum of the whole packet (with header)
		int checksum = 0;
		for(size_t i = 0; i < PacketLen - 1; i++)
			checksum += packet[i];
		packet[PacketLen - 1] = (unsigned char) (checksum & 0xFF);

		if(!SendBlock(port, packet, PacketLen))
			return(false);

		// update header for the next package
		packet[1]++;
		packet[2]--;
	}

	return(true);
}

bool BascomUploader::SendBlock(Port *port, unsigned char *bytes, size_t length)
{
	for(int i = 0; i < BlockMaxRetries && *threadStop == 0; i++) {
		port->Send(bytes, length);

		char response;
		if(!ReadNextByte(port, response))
			return(false);

		if(response == 0x06)
			return(true);

		if(response != 0x15) // if neither ACK nor NAK were send, break with error
			return(false);
	}

	return(false);
}

// waits for the next byte and checks whether is is equal to the argument byte
bool BascomUploader::WaitForByte(Port *port, char byte)
{
	char response;
	return(ReadNextByte(port, response) && response == byte);
}

// waits for the next byte and checks whether is is equal to the argument byte
bool BascomUploader::ReadNextByte(Port *port, char &byte)
{
	for(int tries = 0; tries < MaxTries && *threadStop == 0; tries++) {
		if(port->CanRead()) {
			return(port->ReceiveByte(byte));
		}

		NativeSleep(SleepTime);
	}

	return(false);
}
