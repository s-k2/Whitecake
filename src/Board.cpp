#include "Board.h"

#include "Config.h"

namespace Arduino {

static const std::map<std::string, Address> ioRegisterToAddress = 
{
	{ "PINB", 0x03 },
	{ "DDRB", 0x03 },
	{ "PORTB", 0x05 },
	{ "PINC", 0x06 },
	{ "DDRC", 0x07 },
	{ "PORTC", 0x08 },
	{ "PIND", 0x09 },
	{ "DDRD", 0x0A },
	{ "PORTD", 0x0B }
};

#include "StdlibArduino.h"
}

namespace TinyBas {
static const std::map<std::string, Address> ioRegisterToAddress = 
{
	{ "PINA", 0x19 },
	{ "DDRA", 0x1A },
	{ "PORTA", 0x1B },
	
	{ "PINB", 0x16 },
	{ "DDRB", 0x17 },
	{ "PORTB", 0x18 }
};

#include "StdlibTinyBas.h"
}

namespace TinyTick {

static const std::map<std::string, Address> ioRegisterToAddress = 
{
	{ "PINA", 0x19 },
	{ "DDRA", 0x1A },
	{ "PORTA", 0x1B },
	
	{ "PINB", 0x16 },
	{ "DDRB", 0x17 },
	{ "PORTB", 0x18 }
};

#include "StdlibTinyTick.h"
}

Board::Board()
#ifdef WHITECAKE_FOR_ARDUINO
	: ioRegisterToAddress(Arduino::ioRegisterToAddress), stdlib(Arduino::Stdlib), stdlibSymbols(Arduino::StdlibSymbols)
#endif /* WHITECAKE_FOR_ARDUINO */

#ifdef WHITECAKE_FOR_TINYBAS
	: ioRegisterToAddress(TinyBas::ioRegisterToAddress), stdlib(TinyBas::Stdlib), stdlibSymbols(TinyBas::StdlibSymbols)
#endif /* WHITECAKE_FOR_TINYBAS */

#ifdef WHITECAKE_FOR_TINYTICK
	: ioRegisterToAddress(TinyTick::ioRegisterToAddress), stdlib(TinyTick::Stdlib), stdlibSymbols(TinyTick::StdlibSymbols)
#endif /* WHITECAKE_FOR_TINYTICK */
{
}

