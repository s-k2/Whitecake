#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <map>
#include <string>
#include <vector>

#include "Helper/String.h"
#include "Board.h"

namespace Compiler {

class Program;


class Instruction
{
public:

	uint32_t GetOpcode() const
		{ return(opcode); };
protected:
	Instruction() {}; // abstract base class

	uint32_t opcode;
};

template <uint32_t Opcode, size_t SizeT = 2>
class NoRegisterInstruction : public Instruction
{
public:
	enum { Size = SizeT };

	NoRegisterInstruction()
	{
		opcode = Opcode;
	}
};

static void ApplyMaskedValue(uint32_t &instruction, uint32_t mask, uint32_t value)
{
	for(size_t i = 0; i < sizeof(mask) * 8; i++, mask >>= 1) {
		if((mask & 1) == 1) {
			instruction = (instruction & ~(1 << i)) | ((value & 1) << i);
			value >>= 1;
		}
	}
}

template <uint32_t Opcode, uint32_t OperandMask, size_t SizeT = 2>
class OperandInstruction : public Instruction
{
public:
	enum { Size = SizeT };
	
	OperandInstruction(uint32_t operandValue)
	{
		opcode = Opcode;
		ApplyMaskedValue(opcode, OperandMask, operandValue);
	}

};

template <uint32_t Opcode, uint32_t FirstOperandMask, uint32_t SecondOperandMask, size_t SizeT = 2>
class TwoOperandsInstruction : public Instruction
{
public:
	enum { Size = SizeT };

	TwoOperandsInstruction(uint32_t firstOperandValue, uint32_t secondOperandValue)
	{
		opcode = Opcode;
		ApplyMaskedValue(opcode, FirstOperandMask, firstOperandValue);
		ApplyMaskedValue(opcode, SecondOperandMask, secondOperandValue);
	}
};

typedef NoRegisterInstruction<0b1001010100001000> ReturnInstruction;
typedef NoRegisterInstruction<0b1001010100001001> IndirectCallInstruction;
typedef NoRegisterInstruction<0b1001010000001001> IJMPInstruction;

typedef TwoOperandsInstruction<	0b00000000000000001001000000000000,
				0b00000000000000000000000111110000,
				0b11111111111111110000000000000000, 4> LdsInstruction;

typedef TwoOperandsInstruction<	0b00000000000000001001001000000000,
				0b00000000000000000000000111110000,
				0b11111111111111110000000000000000, 4> StsInstruction;

typedef TwoOperandsInstruction<	0b1110000000000000,
				0b0000000011110000, // only R16-R32 (0 = R16, 15 = R31)
		      		0b0000111100001111> LoadImmediateInstruction; 

typedef TwoOperandsInstruction<	0b0001010000000000,
				0b0000000111110000, 
		      		0b0000001000001111> CompareInstruction; 

typedef TwoOperandsInstruction<	0b0000010000000000,
				0b0000000111110000, 
		      		0b0000001000001111> CompareWithCarryInstruction; 

typedef OperandInstruction<	0b1111010000000100,
				0b0000001111111000> BRGEInstruction; 
typedef OperandInstruction<	0b1111000000000100,
				0b0000001111111000> BRLTInstruction; 
typedef OperandInstruction<	0b1111010000000001,
				0b0000001111111000> BRNEInstruction; 
typedef OperandInstruction<	0b1111000000000001,
				0b0000001111111000> BREQInstruction; 

typedef TwoOperandsInstruction<	0b0001110000000000,
				0b0000000111110000, 
		      		0b0000001000001111> AddWithCarry; 
typedef TwoOperandsInstruction<	0b0000110000000000,
				0b0000000111110000, 
		      		0b0000001000001111> AddWithoutCarry; 
typedef TwoOperandsInstruction<	0b0000100000000000,
				0b0000000111110000, 
		      		0b0000001000001111> SubtractWithCarry; 
typedef TwoOperandsInstruction<	0b0001100000000000,
				0b0000000111110000, 
		      		0b0000001000001111> SubtractWithoutCarry; 
typedef TwoOperandsInstruction<	0b0010100000000000,
				0b0000000111110000, 
		      		0b0000001000001111> OR; 
typedef TwoOperandsInstruction<	0b0010000000000000,
				0b0000000111110000, 
		      		0b0000001000001111> AND; 
typedef TwoOperandsInstruction<	0b0010010000000000,
				0b0000000111110000, 
		      		0b0000001000001111> EOR; 
typedef TwoOperandsInstruction<	0b1001100100000000,
				0b0000000011111000, 
		      		0b0000000000000111> SBIC; 
typedef TwoOperandsInstruction<	0b1111111000000000,
				0b0000000111110000, 
		      		0b0000000000000111> SBRS; 
typedef TwoOperandsInstruction<	0b1111110000000000,
				0b0000000111110000, 
		      		0b0000000000000111> SBRC; 

typedef TwoOperandsInstruction<	0b1001100000000000,
				0b0000000011111000, 
		      		0b0000000000000111> CBI; 
typedef TwoOperandsInstruction<	0b1001101000000000,
				0b0000000011111000, 
		      		0b0000000000000111> SBI; 

typedef TwoOperandsInstruction<	0b0010110000000000,
				0b0000000111110000, 
		      		0b0000001000001111> MOV; 


typedef OperandInstruction<	0b1001010000000011,
				0b0000000111110000> INC;


class IoBit
{
public:
	explicit IoBit(const std::string &portName, uint8_t bit)
		: portName(portName), bit(bit)
	{
	}

	explicit IoBit(const std::string &nameDotBit)
	{
		std::vector<std::string> tokens;
		String::Tokenize(nameDotBit, tokens, '.');
		if(tokens.size() != 2)
			throw std::runtime_error("Invalid Pin specified: " + nameDotBit);

		portName = tokens.front();
	       	bit = std::stoi(tokens.back());
	}

	const std::string &GetPortName() const
		{ return(portName); };
	uint8_t GetBit() const
		{ return(bit); };

private:
	std::string portName;
	uint8_t bit;
};

class UnknownAddress
{
public:
	explicit UnknownAddress(const std::string &globalSymbol)
		: symbol(globalSymbol), offset(0)
	{
	}

	UnknownAddress operator +(int16_t offset) const
	{
		UnknownAddress newAddress = *this;
		newAddress.offset = offset;

		return(newAddress);
	}

	const std::string &GetSymbol() const
		{ return(symbol); };
	int16_t GetOffset() const
		{ return(offset); };

private:
	std::string symbol;
	int16_t offset;
};


class Binary
{
public:
	Binary()
		: currentPos(0)
	{
	}

	template<size_t Size>
	void Append(uint32_t value)
	{
		for(size_t i = 0; i < Size; i++, value >>= 8)
			bytes.push_back(value & 0xff);
		currentPos += Size;
	}

	template<size_t Size>
	void Replace(uint32_t value)
	{
		for(size_t i = 0; i < Size; i++, value >>= 8, currentPos++)
			bytes[currentPos] = value & 0xff;
	}

	void Reserve(size_t size) {
		// if we're in the middle of the code do not reserve anything
		if(currentPos != bytes.size())
			return;

		while(size > 0) {
			bytes.push_back(0x5a);
			size--;
			currentPos++;
		}
	}

	void Seek(size_t newPos) {
		currentPos = newPos;
	}

	void SeekToEnd() {
		currentPos = bytes.size();
	}

	template<class T>
	Binary &operator << (const T &instruction)
	{
		if(bytes.size() == currentPos)
			Append<T::Size>(instruction.GetOpcode());
		else
			Replace<T::Size>(instruction.GetOpcode());

		return(*this);
	}

	uint16_t GetCurrentAddress() const
		{ return((uint16_t) currentPos); };

	void Dump() const
	{
		std::vector<uint8_t> subBytes;
		uint16_t address = 0;
		for(auto byte : bytes) {
			subBytes.push_back(byte);
			if(subBytes.size() == 0x10) {
				DumpLine(subBytes, address);
				subBytes.clear();
				address += 0x10;
			}
		}
		if(subBytes.size() > 0)
			DumpLine(subBytes, address);
	}

	void DumpLine(std::vector<uint8_t> bytes, uint16_t address) const
	{
		printf(":%02X%04X00", (uint8_t) bytes.size(), address);
		size_t checksum = bytes.size() + (address & 0xff) + ((address >> 8) & 0xff);

		for(auto byte : bytes) {
			printf("%02X", byte);
			checksum += byte;
		}
		checksum = -checksum;
		printf("%02X\n", (uint8_t) checksum);
	}

	const std::vector<uint8_t> &GetBinary() const
	{
		return(bytes);
	}


private:
	std::vector<uint8_t> bytes;
	size_t currentPos;
};

class DeferedInstruction
{
public:
	~DeferedInstruction()
		{ };

	virtual void SecondPass(Binary &binary) = 0;

	void SetDataAddress(Address dataAddress)
		{ this->dataAddress = dataAddress + address.GetOffset(); };
	Address GetDataAddress() const
		{ return(dataAddress); };
	const UnknownAddress &GetUnknownAddress() const
		{ return(address); };

protected:
	DeferedInstruction(const UnknownAddress &address)
		: address(address)
	{
	}

	UnknownAddress address;
	Address dataAddress;
};

class DeferedCodeAddressToZ : public DeferedInstruction
{
public:
	DeferedCodeAddressToZ(Binary &binary, const UnknownAddress &address)
		: DeferedInstruction(address), codeAddress(binary.GetCurrentAddress())
	{
		binary.Reserve(LoadImmediateInstruction::Size * 2);
	}

	virtual void SecondPass(Binary &binary)
	{
		binary.Seek(codeAddress);
		binary << LoadImmediateInstruction(14, (GetDataAddress() >> 1) & 0xff) // low-word
			<< LoadImmediateInstruction(15, (GetDataAddress() >> 9) & 0xff); // high-word
	}

private:
	Address codeAddress;
};

class DeferedLoadFlashAddress : public DeferedInstruction
{
public:
	DeferedLoadFlashAddress (Binary &binary, const UnknownAddress &address, uint16_t lowReg, uint16_t highReg)
		: DeferedInstruction(address), codeAddress(binary.GetCurrentAddress()), lowReg(lowReg), highReg(highReg)
	{
		if(highReg < 16 || lowReg < 16)
				throw std::runtime_error("Flash-addresses may not be stored in registers lower than r16");

		binary.Reserve(LoadImmediateInstruction::Size * 2);
	}

	virtual void SecondPass(Binary &binary)
	{
		binary.Seek(codeAddress);
		binary << LoadImmediateInstruction(lowReg, GetDataAddress() & 0xff) // low-word
			<< LoadImmediateInstruction(highReg, (GetDataAddress() >> 8) & 0xff); // high-word
	}

private:
	Address codeAddress;
	uint16_t lowReg;
	uint16_t highReg;
};

template <class T>
class DeferedTwoOperand : public DeferedInstruction
{
public:
	DeferedTwoOperand (Binary &binary, uint16_t registerNumber, const UnknownAddress &address)
		: DeferedInstruction(address), codeAddress(binary.GetCurrentAddress()), registerNumber(registerNumber)
	{
		binary.Reserve(T::Size);
	}

	virtual void SecondPass(Binary &binary)
	{
		binary.Seek(codeAddress);
		binary << T(registerNumber, GetDataAddress());
	}

private:
	Address codeAddress;
	uint16_t registerNumber;
};

typedef DeferedTwoOperand<LdsInstruction> DeferedLDS;
typedef DeferedTwoOperand<StsInstruction> DeferedSTS;

class Expression
{
public:

};

class ValueExpression : public Expression
{
public:
	enum Type { IntLiteralType, StringLiteralType, VariableType, IoBitType } type;
	explicit ValueExpression(int16_t literalInt)
		: type(IntLiteralType), literalInt(literalInt), literalString(""), src("unused"), ioBit("", 0)
	{
	}

	explicit ValueExpression(const std::string &str)
		: type(StringLiteralType), literalInt(0), literalString(str), src("unused"), ioBit("", 0)
	{
	}

	explicit ValueExpression(const UnknownAddress &src)
		: type(VariableType), literalInt(0), literalString(""), src(src), ioBit("", 0)
	{
	}

	explicit ValueExpression(const IoBit &ioBit)
		: type(IoBitType), literalInt(0), literalString(""), src("unused"), ioBit(ioBit)
	{
	}

	Type GetType() const
		{ return(type); };
	int16_t GetLiteralIntValue() const
		{ return(literalInt); };
	const std::string &GetLiteralStringValue() const
		{ return(literalString); };
	const UnknownAddress &GetSource() const
		{ return(src); };
	const IoBit &GetIoBit() const
		{ return(ioBit); };

private:
	int16_t literalInt;
	std::string literalString;
	UnknownAddress src;
	IoBit ioBit;
};

class MathExpression : public Expression
{
public:
	enum Operator { Plus, Minus, Multiply, Divide, BitOr, BitAnd };
	explicit MathExpression (const ValueExpression  &left, Operator op, const ValueExpression &right)
		: left(left), op(op), right(right)
	{
	}

	const ValueExpression &GetLeft() const
		{ return(left); };
	const ValueExpression &GetRight() const
		{ return(right); };
	Operator GetOperator() const
		{ return(op); };

private:
	ValueExpression left;
	Operator op;
	ValueExpression right;
};

class Program
{
public:
	Program();

	void Call(const UnknownAddress &address)
	{
		deferedInstructions.emplace_back(new DeferedCodeAddressToZ(binary, address));
		binary << IndirectCallInstruction();
	}

	void Call(const UnknownAddress &address, const std::vector<ValueExpression> args)
	{
		size_t currentRegisterArg = 24;
		for(auto &arg : args) {
			CodifyExpression(arg, currentRegisterArg, currentRegisterArg + 1);
			currentRegisterArg -= 2;
		}

		Call(address);
	}

	void Call(const UnknownAddress &address, const std::vector<ValueExpression> args, const UnknownAddress &retVar)
	{
		Call(address, args);

		deferedInstructions.emplace_back(new DeferedSTS(binary, 24, retVar));
		deferedInstructions.emplace_back(new DeferedSTS(binary, 25, retVar + 1));
	}

	void ReturnFunction()
	{
		binary << ReturnInstruction();
	}

	template<class ExpressionT>
	void Assign(const UnknownAddress &dest, const ExpressionT &src)
	{
		CodifyExpression(src); // result is in 24, 25

		deferedInstructions.emplace_back(new DeferedSTS(binary, 24, dest));
		deferedInstructions.emplace_back(new DeferedSTS(binary, 25, dest + 1));
	}
	
	void Assign(const IoBit &dest, const MathExpression &src)
	{
		CodifyExpression(src); // result is in 24, 25

		binary << SBRS(24, 0);
		binary << CBI(board.GetAddressOfIo(dest.GetPortName()), dest.GetBit());
		binary << SBRC(24, 0);
		binary << SBI(board.GetAddressOfIo(dest.GetPortName()), dest.GetBit());
	}

	void Assign(const IoBit &dest, const ValueExpression &src)
	{
		if(src.GetType() == ValueExpression::IntLiteralType) {
			if(src.GetLiteralIntValue() == 0)
				binary << CBI(board.GetAddressOfIo(dest.GetPortName()), dest.GetBit());
			else if(src.GetLiteralIntValue() == 1)
				binary << SBI(board.GetAddressOfIo(dest.GetPortName()), dest.GetBit());
			else
				throw std::runtime_error("Invalid literal-value for IoBit (only 0 and 1 allowed)");

		} else {
			CodifyExpression(src); // result is in 24, 25

			binary << SBRS(24, 0);
			binary << CBI(board.GetAddressOfIo(dest.GetPortName()), dest.GetBit());
			binary << SBRC(24, 0);
			binary << SBI(board.GetAddressOfIo(dest.GetPortName()), dest.GetBit());
		}
	}

	enum CompareMode { Greater, GreaterEqual, Equal, Lower, LowerEqual, NotEqual };
	void CompareAndBranch(const ValueExpression &left, CompareMode  compareMode, const ValueExpression &right, const UnknownAddress &falseBranch)
	{
		CodifyExpression(left, 24, 25);
		CodifyExpression(right, 20, 21);

		// those modes not natively supported by the avr can be eumulated by changing the direction of the compared registers
		if(compareMode == LowerEqual || compareMode == Greater) {
			compareMode = (compareMode == LowerEqual) ? GreaterEqual : Lower;
			binary << CompareInstruction(20, 24);
			binary << CompareWithCarryInstruction(21, 25);
		} else {
			binary << CompareInstruction(24, 20);
			binary << CompareWithCarryInstruction(25, 21);
		}

		static const size_t JumpToFalseSize = (2 * LoadImmediateInstruction::Size + IJMPInstruction::Size) / 2;
		switch(compareMode) {
			case Lower:
				binary << BRLTInstruction(JumpToFalseSize);
				break;
			case Equal:
				binary << BREQInstruction(JumpToFalseSize);
				break;
			case NotEqual:
				binary << BRNEInstruction(JumpToFalseSize);
				break;
			case GreaterEqual:
				binary << BRGEInstruction(JumpToFalseSize);
				break;
			default:
				throw std::runtime_error("Yet unsupported CompareMode encountered");
				break;
		}

		deferedInstructions.emplace_back(new DeferedCodeAddressToZ(binary, falseBranch));
		binary << IJMPInstruction();
	}

	void LocalJump(const UnknownAddress &destination)
	{
		deferedInstructions.emplace_back(new DeferedCodeAddressToZ(binary, destination));
		binary << IJMPInstruction();
	}

	inline std::string GenerateSymbolName()
		{ return(std::string("loc") + std::to_string(++nextSymbolId)); };

	void LabelLocation(const std::string &name, Address address)
	{
		labels.insert(std::make_pair(name, address));
	}

	std::string LabelCurrentLocation()
	{
		std::string name = GenerateSymbolName();
		labels.insert(std::make_pair(name, binary.GetCurrentAddress()));
		return(name);
	}
	void LabelCurrentLocation(const std::string &name)
	{
		labels.insert(std::make_pair(name, binary.GetCurrentAddress()));
	}

	Address GetLocationByLabel(const std::string &name) const
	{
		auto address = labels.find(name);
		if(address == labels.end())
			throw std::runtime_error("Location " + name + " has not been defined");
		return(address->second);
	}

	void Finish()
	{
		for(auto &literal : literalStrings) {
			LabelCurrentLocation(literal.first);
			for(char ch : literal.second)
				binary.Append<1>((uint8_t) ch);
			binary.Append<1>(0x00); // mark the end
		}

		for(auto &instruction : deferedInstructions) {
			Address addr = GetLocationByLabel(instruction->GetUnknownAddress().GetSymbol());
			instruction->SetDataAddress(addr);
			instruction->SecondPass(binary);
		}
	}

	void Dump() const
	{
		binary.Dump();
		printf("\n");
	}

	const std::vector<uint8_t> &GetBinary() const
	{
		return(binary.GetBinary());
	}

private:
	Board board;
	Binary binary;
	size_t nextSymbolId;

	std::map<std::string, Address> labels;
	std::vector<std::pair<std::string, std::string>> literalStrings;
	std::vector<std::unique_ptr<DeferedInstruction>> deferedInstructions; // TODO: Tidy up


	void CodifyExpression(const MathExpression &mathExpression);
	void CodifyExpression(const ValueExpression &valueExpression, uint16_t lowReg = 24, uint16_t highReg = 25);
};

}

#endif /* COMPILER_H */
