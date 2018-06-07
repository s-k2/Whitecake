#include "Compiler.h"

#include "Config.h"

namespace Compiler {

Microcontroller::Microcontroller()
{
/*	Arduino
 */
#ifdef WHITECAKE_FOR_ARDUINO
	ioRegisterToAddress.insert(std::make_pair("PINB", 0x03));
	ioRegisterToAddress.insert(std::make_pair("DDRB", 0x03));
	ioRegisterToAddress.insert(std::make_pair("PORTB", 0x05));
	ioRegisterToAddress.insert(std::make_pair("PINC", 0x06));
	ioRegisterToAddress.insert(std::make_pair("DDRC", 0x07));
	ioRegisterToAddress.insert(std::make_pair("PORTC", 0x08));
	ioRegisterToAddress.insert(std::make_pair("PIND", 0x09));
	ioRegisterToAddress.insert(std::make_pair("DDRD", 0x0A));
	ioRegisterToAddress.insert(std::make_pair("PORTD", 0x0B));
#endif /* WHITECAKE_FOR_ARDUINO */

/* TinyBas
 */
#ifdef WHITECAKE_FOR_TINYBAS
	ioRegisterToAddress.insert(std::make_pair("PINA", 0x19));
	ioRegisterToAddress.insert(std::make_pair("DDRA", 0x1A));
	ioRegisterToAddress.insert(std::make_pair("PORTA", 0x1B));
	
	ioRegisterToAddress.insert(std::make_pair("PINB", 0x16));
	ioRegisterToAddress.insert(std::make_pair("DDRB", 0x17));
	ioRegisterToAddress.insert(std::make_pair("PORTB", 0x18));
#endif /* WHITECAKE_FOR_TINYBAS */

/* TinyTick
 */
#ifdef WHITECAKE_FOR_TINYTICK
	ioRegisterToAddress.insert(std::make_pair("PINA", 0x19));
	ioRegisterToAddress.insert(std::make_pair("DDRA", 0x1A));
	ioRegisterToAddress.insert(std::make_pair("PORTA", 0x1B));
	
	ioRegisterToAddress.insert(std::make_pair("PINB", 0x16));
	ioRegisterToAddress.insert(std::make_pair("DDRB", 0x17));
	ioRegisterToAddress.insert(std::make_pair("PORTB", 0x18));
#endif /* WHITECAKE_FOR_TINYTICK */
}


Program::Program()
{
#ifdef WHITECAKE_FOR_ARDUINO
#include "StdlibArduino.h"
#endif /* WHITECAKE_FOR_ARDUINO */

#ifdef WHITECAKE_FOR_TINYBAS
#include "StdlibTinyBas.h"
#endif /* WHITECAKE_FOR_TINYBAS */

#ifdef WHITECAKE_FOR_TINYTICK
#include "StdlibTinyTick.h"
#endif /* WHITECAKE_FOR_TINYTICK */
}

void Program::CodifyExpression(const MathExpression &mathExpression)
{
	CodifyExpression(mathExpression.GetLeft(), 24, 25);
	CodifyExpression(mathExpression.GetRight(), 22, 23);

	switch(mathExpression.GetOperator()) {
		case MathExpression::Plus:
			binary << AddWithoutCarry(24, 22);
			binary << AddWithCarry(25, 23);
			break;
		case MathExpression::Minus:
			binary << SubtractWithoutCarry(24, 22);
			binary << SubtractWithCarry(25, 23);
			break;

		case MathExpression::Multiply:
			deferedInstructions.emplace_back(new DeferedCodeAddressToZ(binary, UnknownAddress("__mulhi3")));
			binary << IndirectCallInstruction();
			break;

		case MathExpression::Divide:
			deferedInstructions.emplace_back(new DeferedCodeAddressToZ(binary, UnknownAddress("__divmodhi4")));
			binary << IndirectCallInstruction();
			binary << MOV(24, 22);
			binary << MOV(25, 23);
			break;
			
		default:
			throw std::runtime_error("Operation not yet supported");
			break;
	}
}

void Program::CodifyExpression(const ValueExpression &valueExpression, uint16_t lowReg, uint16_t highReg)
{
	if(valueExpression.GetType() == ValueExpression::IntLiteralType) {
		if(highReg < 16 || lowReg < 16)
			throw std::runtime_error("LiteralExpresion may not be stored in registers lower than r16");

		binary << LoadImmediateInstruction(lowReg - 16, valueExpression.GetLiteralIntValue() & 0xff);
		binary << LoadImmediateInstruction(highReg - 16, (valueExpression.GetLiteralIntValue() >> 8) & 0xff);
	} else if(valueExpression.GetType() == ValueExpression::StringLiteralType) {
		std::string literalSymbol = GenerateSymbolName();
		literalStrings.push_back(std::make_pair(literalSymbol, valueExpression.GetLiteralStringValue()));
		deferedInstructions.emplace_back(new DeferedLoadFlashAddress(binary, UnknownAddress(literalSymbol), lowReg, highReg));
	} else if(valueExpression.GetType() == ValueExpression::VariableType) {
		deferedInstructions.emplace_back(new DeferedLDS(binary, lowReg, valueExpression.GetSource()));
		deferedInstructions.emplace_back(new DeferedLDS(binary, highReg, valueExpression.GetSource() + 1));
	} else {
		binary << EOR(lowReg, lowReg);
		binary << EOR(highReg, highReg);
		binary << SBIC(microcontroller.GetAddressOfIo(valueExpression.GetIoBit().GetPortName()), 
				valueExpression.GetIoBit().GetBit());
		binary << INC(lowReg);
	}
}

}
