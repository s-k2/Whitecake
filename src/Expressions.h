#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <string>
#include <vector>

#include "Variables.h"

#include "Compiler.h"

class RightSideToken
{
public:
	RightSideToken();
	RightSideToken(const Variables &variables, const std::string &str);

	std::string GetString() const;
	int GetType() const;

	inline int GetLiteralValue() const
		{ return(literalValue); };

	inline bool IsVariable() const
		{ return(IsValid() && variable != NULL); };
	inline bool IsLiteral() const
		{ return(IsValid() && variable == NULL); };
	inline bool IsValid() const
		{ return(isValid); };

	Compiler::ValueExpression ToValueExpression() const
	{
		if(IsLiteral())
			return(Compiler::ValueExpression((uint16_t) GetLiteralValue()));
		else
			return(ToValueExpression(
				variable->GetTarget() != NULL ? variable->GetTarget() : variable));
	}

	Compiler::ValueExpression ToValueExpression(Variable *variable) const
	{
		if(variable->GetType() == IntegerVariable)
			return(Compiler::ValueExpression(Compiler::UnknownAddress(variable->GetName())));
		else if(variable->GetType() == IOPin)
			return(Compiler::ValueExpression(Compiler::IoBit(variable->GetName())));
		else if(variable->GetType() == ADCChannel)
			return(Compiler::ValueExpression((uint16_t) std::stoi(variable->GetName().substr(3))));
		else
			throw std::runtime_error("Unknown variable " + variable->GetName());
	}

private:
	Variable *variable;
	int literalValue;
	bool isValid;
};

class RightSide
{
public:
	RightSide(); // creates an invalid one, that could be assigned with an valid
	RightSide(const Variables &variables, const std::string &expression);
	
	std::string GetString() const;
	int GetType() const;

	inline bool IsValid() const
		{ return(left.IsValid() && (op == NoOperation || right.IsValid())); };

	bool IsMathExpression() const
		{ return(op != NoOperation); };
	Compiler::ValueExpression ToValueExpression() const
	{
		if(op != NoOperation)
			throw std::runtime_error("ToValueExpression() called, but item is no ValueExpression");

		return(left.ToValueExpression());
	}

	Compiler::MathExpression ToMathExpression() const
	{
		if(op == NoOperation)
			throw std::runtime_error("ToMathExpression() called, but item is ValueExpression");

		Compiler::MathExpression::Operator mathOperator = Compiler::MathExpression::Plus;
		if(op == Plus)
			mathOperator = Compiler::MathExpression::Plus;
		else if(op == Minus)
			mathOperator = Compiler::MathExpression::Minus;
		else if(op == Multiplication)
			mathOperator = Compiler::MathExpression::Multiply;
		else if(op == Division)
			mathOperator = Compiler::MathExpression::Divide;

		return(Compiler::MathExpression(left.ToValueExpression(),
					mathOperator,
					right.ToValueExpression()));
	}

private:
	RightSideToken left;
	enum Operator { Plus, Minus, Multiplication, Division, NoOperation };
	Operator op;
	RightSideToken right;
};

class CompareOperator
{
public:
	enum Operator { Below, BelowEqual, Equal, AboveEqual, Above, NotEqual, NoOperator };

	CompareOperator()
		: op(NoOperator)
	{
	}

	bool IsValid() const
		{ return(op != NoOperator); };

	void Parse(const std::string &str)
	{
		if(str == "<")
			op = Below;
		else if(str == "<=")
			op = BelowEqual;
		else if(str == "=")
			op = Equal;
		else if(str == ">=")
			op = AboveEqual;
		else if(str == ">")
			op = Above;
		else if(str == "<>")
			op = NotEqual;
		else
			op = NoOperator;
	}

	const std::string &GetString() const
	{
		static const std::string compareTexts[] = { "<", "<=", "=", ">=", ">", "<>", "?" };
		return(compareTexts[op]);
	}

	const Compiler::Program::CompareMode GetCompilerCompareMode() const
	{
		switch(op) {
			case Below:
				return(Compiler::Program::Lower);
			case BelowEqual:
				return(Compiler::Program::LowerEqual);
			case Equal:
				return(Compiler::Program::Equal);
			case AboveEqual:
				return(Compiler::Program::GreaterEqual);
			case Above:
				return(Compiler::Program::Greater);
			case NotEqual:
				return(Compiler::Program::NotEqual);
			default:
				throw std::runtime_error("Invalid CompareMode cannot be prepared for compiler");
		}
	}

	CompareOperator operator =(const Operator &right)
	{
		op = right;
		return(*this);
	}

private:
	Operator op;
};


class ParsedAssignment
{
public:
	ParsedAssignment(const Variables &variables, const std::string &expression);
	ParsedAssignment();

	inline bool IsLeftSideValid() const
		{ return(leftSide != NULL); };
	inline bool IsRightSideValid() const
		{ return(rightSide.IsValid()); };

	inline bool IsValid() const
		{ return(IsLeftSideValid() && IsRightSideValid() &&
			variables->GetCastableTypes(leftSide->GetType()) & rightSide.GetType()); };

	inline Variable *GetLeftSide() const
		{ return(leftSide); };
	inline const RightSide &GetRightSide() const
		{ return(rightSide); };
	
	// converts the whole assignment to an string
	std::string GetString(const std::string whitespace = " ") const;

	std::vector<std::string> SuggestCompletions() const;

	void WriteCode(Compiler::Program &program) const
	{
		Variable *dest = leftSide->GetTarget() == NULL ? leftSide : leftSide->GetTarget();
		
		if(dest->GetType() == IntegerVariable && rightSide.IsMathExpression())
			program.Assign(Compiler::UnknownAddress(dest->GetName()), 
					rightSide.ToMathExpression());
		else if(dest->GetType() == IOPort && rightSide.IsMathExpression())
			program.Assign(Compiler::IoBit(dest->GetName()), 
					rightSide.ToMathExpression());
		else if(dest->GetType() == IntegerVariable && !rightSide.IsMathExpression())
			program.Assign(Compiler::UnknownAddress(dest->GetName()), 
					rightSide.ToValueExpression());
		else if(dest->GetType() == IOPort && !rightSide.IsMathExpression())
			program.Assign(Compiler::IoBit(dest->GetName()), rightSide.ToValueExpression());
	}

private:
	Variable *leftSide;
	RightSide rightSide;
	Variables *variables;
};

class ParsedCompare
{
public:
	ParsedCompare(const Variables &variables, const std::string &expression);
	ParsedCompare(); // TODO: REMOVE MEEEEEEEE

	inline bool IsLeftSideValid() const
		{ return(leftSide.IsValid()); };
	inline bool IsRightSideValid() const
		{ return(rightSide.IsValid()); };
	inline bool HasCompareOperator() const
		{ return(compareOperator.IsValid()); };

	inline bool IsValid() const
		{ return(leftSide.IsValid() && rightSide.IsValid()); };

	inline const RightSideToken &GetLeftSide() const
		{ return(leftSide); };
	inline const CompareOperator GetCompareOperator() const
		{ return(compareOperator); };
	inline const RightSideToken &GetRightSide() const
		{ return(rightSide); };
	
	// converts the whole compare to an string
	std::string GetString(const std::string whitespace = " ") const;
	void WriteCode(Compiler::Program &program, const std::string &falseBranch) const
	{
		program.CompareAndBranch(leftSide.ToValueExpression(), 
				compareOperator.GetCompilerCompareMode(), 
				rightSide.ToValueExpression(), 
				Compiler::UnknownAddress(falseBranch));
	}

	std::vector<std::string> SuggestCompletion() const;

private:
	RightSideToken leftSide;
	RightSideToken rightSide;
	CompareOperator compareOperator;
	Variables *variables;
};

class ParsedParameter
{
public:
	ParsedParameter(const Variables &variables, std::string token);

	bool IsValid(int filter) const;

	const std::string &GetText() const;

	Compiler::ValueExpression ToValueExpression()
	{
		if(type == StringLiteral)
			return(Compiler::ValueExpression(strLiteralValue));
		else if(type == NumLiteral)
			return(Compiler::ValueExpression((uint16_t) numLiteralValue));
		else
			return(ToValueExpression(
				variable->GetTarget() != NULL ? variable->GetTarget() : variable));
	}

	Compiler::ValueExpression ToValueExpression(Variable *variable) const
	{
		if(variable->GetType() == IntegerVariable)
			return(Compiler::ValueExpression(Compiler::UnknownAddress(variable->GetName())));
		else if(variable->GetType() == IOPin)
			return(Compiler::ValueExpression(Compiler::IoBit(variable->GetName())));
		else if(variable->GetType() == ADCChannel)
			return(Compiler::ValueExpression((uint16_t) std::stoi(variable->GetName().substr(3))));
		else
			throw std::runtime_error("Unknown variable " + variable->GetName());
	}

private:
	std::string token;
	enum Type { VariableName, NumLiteral, StringLiteral, Invalid };
	Type type;
	Variable *variable;
	int numLiteralValue;
	std::string strLiteralValue;

	bool IsValidNumLiteral(int filter) const;
	bool IsValidStringLiteral(int filter) const;
	bool IsValidVariable(int filter) const;
};

#endif /* EXPRESSIONS_H */
