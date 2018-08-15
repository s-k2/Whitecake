#include "Compiler.h"

#include "Config.h"

namespace Compiler {

Program::Program()
{
	// copy all bytes of the stdlib-binary and all symbols of it
	for(auto byte : board.GetStdlib())
		binary.Append<1>(byte);
	labels = board.GetStdlibSymbols();

	// label the first byte of our own code and load this address at location call_user_main
	LabelCurrentLocation("program_start");
	Address callUserMainAddress = GetLocationByLabel("call_user_main");
	binary.Seek(callUserMainAddress);
	deferedInstructions.emplace_back(new DeferedCodeAddressToZ(binary, UnknownAddress("program_start")));
	binary.SeekToEnd();	
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
		binary << SBIC(board.GetAddressOfIo(valueExpression.GetIoBit().GetPortName()), 
				valueExpression.GetIoBit().GetBit());
		binary << INC(lowReg);
	}
}

}
