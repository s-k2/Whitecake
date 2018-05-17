#include "Microcontroller.h"

#include <time.h>
#include <string>
using std::string;

#include "Helper/BasicWriter.h"
#include "Helper/XMLReader.h"
#include "Helper/XMLWriter.h"
#include "Settings.h"

Microcontroller::Microcontroller()
{
	regFileName = theSettings.GetRegFileName();
	crystalFrequency = theSettings.GetCrystalFreqency();
	hardwareStack = theSettings.GetHardwareStack();
	softwareStack = theSettings.GetSoftwareStack();
	frameSize = theSettings.GetFrameSize();

	usingHardwareUART = theSettings.GetUsingHardwareUART();
	hardwareUARTBaud = theSettings.GetSerialBaud();
	softUARTIn = theSettings.GetSoftUARTIn();
	softUARTOut = theSettings.GetSoftUARTOut();
}

Microcontroller::Microcontroller(XMLReader *xml)
{
	xml->OpenTag("Microcontroller");
	
	xml->TextTag("RegFile", &regFileName);
	xml->TextTag("CrystalFrequency", &crystalFrequency);
	xml->TextTag("HardwareStack", &hardwareStack);
	xml->TextTag("SoftwareStack", &softwareStack);
	xml->TextTag("FrameSize", &frameSize);
	string usingHardwareUARTStr;
	xml->TextTag("UsingHardwareUART", &usingHardwareUARTStr);
	if(usingHardwareUARTStr == "true")
		usingHardwareUART = true;
	else
		usingHardwareUART = false;
	xml->TextTag("SoftUARTIn", &softUARTIn);
	xml->TextTag("SoftUARTOut", &softUARTOut);

	xml->TextTag("Baudrate", &hardwareUARTBaud);

	variables.Read(xml);

	xml->CloseTag("Microcontroller");
}

void Microcontroller::WriteXML(XMLWriter *xml)
{
	xml->OpenTag("Microcontroller");
	
	xml->TextTag("RegFile", regFileName);
	xml->TextTag("CrystalFrequency", crystalFrequency);
	xml->TextTag("HardwareStack", hardwareStack);
	xml->TextTag("SoftwareStack", softwareStack);
	xml->TextTag("FrameSize", frameSize);
	xml->TextTag("UsingHardwareUART", usingHardwareUART ? "true" : "false");
	xml->TextTag("SoftUARTIn", softUARTIn);
	xml->TextTag("SoftUARTOut", softUARTOut);
	xml->TextTag("Baudrate", hardwareUARTBaud);

	variables.Write(xml);

	xml->CloseTag("Microcontroller");
}

Microcontroller::~Microcontroller()
{
}

void Microcontroller::WriteBasicHeader(BasicWriter *writer)
{
	time_t currentTime = time(NULL);
	char dateStr[32];
	strftime(dateStr, 32, "%c", localtime(&currentTime)); // TODO: Find a solution for the localtime()-thread-safety problem
	writer->PutCodeVarArgs("' This file was created on %s by Whitecake", dateStr);
	
	writer->PutCode("");

	writer->PutCodeVarArgs("$regfile = \"%s\"", regFileName.c_str());
	writer->PutCodeVarArgs("$crystal = %Iu", crystalFrequency);

	writer->PutCodeVarArgs("$hwstack = %d", hardwareStack);
	writer->PutCodeVarArgs("$swstack = %d", softwareStack);
	writer->PutCodeVarArgs("$framesize = %d", frameSize);

	writer->PutCode("");
	
	writer->PutCode("' Define all variables and ports");

	if(usingHardwareUART) {
		writer->PutCodeVarArgs("$baud = %d", hardwareUARTBaud);
		writer->PutCode("Config COM1 = Dummy, Synchrone = 0, Parity = None, Stopbits = 1, Databits = 8, Clockpol = 0");
	}

	// It is very important that the Software-UART is initialized after all
	// variables and ports as it is normal code that will be executed
	if(!softUARTOut.empty() && !softUARTIn.empty()) {
		writer->PutCodeVarArgs("Open \"%s\" For Output As #1", softUARTOut.c_str());
		writer->PutCodeVarArgs("Open \"%s\" For Input As #2", softUARTIn.c_str());
	}

	variables.WriteBasicHeader(writer);


	writer->PutCode("");
}