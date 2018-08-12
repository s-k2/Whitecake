#include "Microcontroller.h"

#include <time.h>
#include <string>
using std::string;

#include "Helper/XMLReader.h"
#include "Helper/XMLWriter.h"
#include "Settings.h"

Microcontroller::Microcontroller()
{
}

Microcontroller::Microcontroller(XMLReader *xml)
{
	xml->OpenTag("Microcontroller");
	
	variables.Read(xml);

	xml->CloseTag("Microcontroller");
}

void Microcontroller::WriteXML(XMLWriter *xml)
{
	xml->OpenTag("Microcontroller");
	
	variables.Write(xml);

	xml->CloseTag("Microcontroller");
}

Microcontroller::~Microcontroller()
{
}

