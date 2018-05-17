#ifndef MICROCONTROLLER_H
#define MICROCONTROLLER_H

#include <string>

#include "Variables.h"

class BasicWriter;
class XMLReader;
class XMLWriter;

class Microcontroller
{
public:
	Microcontroller();
	Microcontroller(XMLReader *reader);
	~Microcontroller();

	inline const Variables &GetVariables() const
		{ return(variables); };
	inline Variables &GetVariables()
		{ return(variables); };

	void WriteBasicHeader(BasicWriter *writer);

	//bool ValidateExpression(const std::string &str, int filter, std::string &errorMessage);

	void WriteXML(XMLWriter *xml);
	
	inline std::string GetSoftUARTOutFile() const
		{ return(softUARTOut.empty() ? std::string("") : std::string("#1")); };
	inline std::string GetSoftUARTInFile() const
		{ return(softUARTIn.empty() ? std::string("") : std::string("#2")); };
	inline const std::string &GetRegFileName() const
		{ return(regFileName); };

private:
	Variables variables;

	std::string regFileName;
	bool usingHardwareUART;
	int hardwareUARTBaud;
	bool usingSoftUART;
	std::string softUARTIn;
	std::string softUARTOut;
	int crystalFrequency;
	int hardwareStack;
	int softwareStack;
	int frameSize;
};

#endif /* MICROCONTROLLER_H */