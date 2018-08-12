#ifndef MICROCONTROLLER_H
#define MICROCONTROLLER_H

#include <string>

#include "Variables.h"

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

	void WriteXML(XMLWriter *xml);
	
private:
	Variables variables;
};

#endif /* MICROCONTROLLER_H */
