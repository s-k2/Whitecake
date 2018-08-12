#include "Expressions.h"

#include "Helper/String.h"
using std::string;
using std::vector;
#include <functional>
using std::find;

RightSideToken::RightSideToken()
	: variable(NULL), literalValue(0), isValid(false)
{
}

RightSideToken::RightSideToken(const Variables &variables, const std::string &str)
	: variable(NULL), isValid(true)
{
	if(String::FirstCharLikeNumber(str, "&H")) {
		if(!String::ToInt(str, literalValue, "&H"))
			isValid = false;
	} else {
		if((variable = variables.GetOperand(str)) == NULL)
			isValid = false;
	}
}

std::string RightSideToken::GetString() const
{
	if(IsVariable())
		return(variable->GetName());
	else if(IsLiteral())
		return(String::IntToStr(literalValue));
	else
		return("");
}

int RightSideToken::GetType() const
{
	if(IsVariable()) {
		return(variable->GetType());
	} else if(IsLiteral()) {
		if(GetLiteralValue() == 0 || GetLiteralValue() == 1)
			return(FixedBit);
		else if(GetLiteralValue() >= -128 && GetLiteralValue() <= 127)
			return(FixedByte);
		else
			return(FixedInteger);
	} else {
		return(0);
	}
}

RightSide::RightSide()
	: op(NoOperation)
{
}

RightSide::RightSide(const Variables &variables, const string &expression)
{
	string firstToken, secondToken;
	op = NoOperation;

	enum State { WaitFirstToken, InFirstToken, WaitOp, InOp, WaitSecondToken, InSecondToken, ExpectNothing } state = WaitFirstToken;
	for(auto it = expression.begin(); it != expression.end(); ++it) {
		switch(state) {
			case WaitFirstToken:
				if(!isspace(*it)) {
					state = InFirstToken;
					goto InFirstTokenLabel;
				}
				break;
InFirstTokenLabel:
			case InFirstToken:
				if(*it == '+' || (*it == '-' && !firstToken.empty()) || *it == '*' || *it == '/') {
					state = InOp;
					goto InOpLabel;
				} else if(isspace(*it)) {
					state = WaitOp;
					break;
				}
				firstToken += *it;
				break;

			case WaitOp:
				if(!isspace(*it)) {
					state = InOp;
					goto InOpLabel;
				}
				break;

InOpLabel:
			case InOp:
				if(*it == '+') {
					op = Plus;
				} else if(*it == '-') { // it's only a minus if it follows the first token
					op = Minus;
				} else if(*it == '*') {
					op = Multiplication;
				} else if(*it == '/') {
					op = Division;
				}
				state = WaitSecondToken;
				break;

			case WaitSecondToken:
				if(!isspace(*it)) {
					state = InSecondToken;
					goto InSecondTokenLabel;
				}
				break;

InSecondTokenLabel:
			case InSecondToken:
				if(isspace(*it)) {
					state = ExpectNothing;
					goto ExpectNothingLabel;
				}
				secondToken += *it;
				break;

ExpectNothingLabel:
			case ExpectNothing:
				break;
		}
	}

	left = RightSideToken(variables, firstToken);
	if(!secondToken.empty())
		right = RightSideToken(variables, secondToken);
}

int RightSide::GetType() const
{
	if(op == NoOperation)
		return(left.GetType());
	else
		return(left.GetType() | right.GetType()); // TODO: How could we handle the different operations better?
}

std::string RightSide::GetString() const
{
	static const char *opCh = "+-*/";
	if(op != NoOperation)
		return(left.GetString() + string(" ") + opCh[op] + string(" ") + right.GetString());
	else
		return(left.GetString());
}


ParsedAssignment::ParsedAssignment(const Variables &variables, const std::string &input)
	: variables((Variables *) &variables)
{
	size_t equalsSignPos = input.find_first_of('=');

	string leftSideUntrimmed = input.substr(0, equalsSignPos);
	string leftSideStr = String::Trim(leftSideUntrimmed);

	string rightSideStr;
	if(equalsSignPos != string::npos && equalsSignPos + 1 != input.size()) {
		rightSideStr = input.substr(equalsSignPos + 1, string::npos);
		String::Trim(rightSideStr);
	}
	
	leftSide = variables.GetOperand(leftSideStr);
	if(!rightSideStr.empty())
		rightSide = RightSide(variables, rightSideStr);
}

ParsedAssignment::ParsedAssignment()
	: leftSide(NULL)
{
}

string ParsedAssignment::GetString(const string whitespace) const
{
	if(IsLeftSideValid() && IsRightSideValid())
		return(GetLeftSide()->GetName() + whitespace + "=" + whitespace + GetRightSide().GetString());
	else if(IsLeftSideValid() && !IsRightSideValid())
		return(GetLeftSide()->GetName() + whitespace + "=" + whitespace);
	else if(!IsLeftSideValid() && IsRightSideValid())
		return(whitespace + "=" + whitespace + GetRightSide().GetString());
	else
		return("");
}


vector<string> ParsedAssignment::SuggestCompletions() const
{
	static const int AssignmentLeftFilter = IntegerVariable | CharVariable | IOPort;
	
	if(!IsLeftSideValid()) {
		return(variables->FormatOperands(AssignmentLeftFilter, "", " = "));
	} else {
		// we just show those variables that fit into the left side
		int contextualRightFilter = variables->GetCastableTypes(GetLeftSide()->GetType());
		return(variables->FormatOperands(contextualRightFilter, GetLeftSide()->GetName() + " = "));
	}
}

ParsedCompare::ParsedCompare(const Variables &variables, const std::string &text)
	: variables((Variables *) &variables)
{
	size_t compareSignPos = text.find_first_of("<=>");

	string leftSideStr = text.substr(0, compareSignPos);
	String::Trim(leftSideStr);

	string compareSignStr;
	string rightSideStr;

	// if the sign exists and one char after that sign
	if(compareSignPos != string::npos && compareSignPos + 1 != text.size()) {
		// if the compare sign is either <= >=
		if(text.substr(compareSignPos, 2) == "<=" || text.substr(compareSignPos, 2) == ">=") {
			compareSignStr = text.substr(compareSignPos, 2);
			// if there is any char on the right side
			if(compareSignPos + 2 != text.size())
				rightSideStr = text.substr(compareSignPos + 2);
		} else {
			compareSignStr = text.substr(compareSignPos, 1);
			// we've already checked if there is at least one char of the right side
			rightSideStr = text.substr(compareSignPos + 1);
		}
	} else if(compareSignPos != string::npos) {
		compareSignStr = text.substr(compareSignPos, 1); // at least the sign exists
	}

	String::Trim(compareSignStr);
	String::Trim(rightSideStr);

	leftSide = RightSideToken(variables, leftSideStr);
	compareOperator.Parse(compareSignStr);
	rightSide = RightSideToken(variables, rightSideStr);
}

ParsedCompare::ParsedCompare()
{
}

string ParsedCompare::GetString(const std::string whitespace) const
{
	string text;
	if(leftSide.IsValid())
		text = leftSide.GetString() + whitespace;
	if(compareOperator.IsValid())
		text += compareOperator.GetString() + whitespace;
	if(rightSide.IsValid())
		text += rightSide.GetString();

	return(text);
}

vector<string> ParsedCompare::SuggestCompletion() const
{
	static const int LeftFilter = IntegerVariable | CharVariable | IOPort | IOPin;

	if(!IsLeftSideValid()) {
		return(variables->FormatOperands(LeftFilter));
	} else if(!HasCompareOperator()) {
		vector<string> suggestions;
		suggestions.push_back(GetLeftSide().GetString() + " > ");
		suggestions.push_back(GetLeftSide().GetString() + " >= ");
		suggestions.push_back(GetLeftSide().GetString() + " = ");
		suggestions.push_back(GetLeftSide().GetString() + " < ");
		suggestions.push_back(GetLeftSide().GetString() + " <= ");
		return(suggestions);
	} else {
		// we just show, what can be stored in the left-side
		int contextualRightFilter;
 		if(GetLeftSide().IsLiteral()) // if the left side was a number, we can compare with everything
			contextualRightFilter = LeftFilter;
		else
			contextualRightFilter = variables->GetCastableTypes(GetLeftSide().GetType()); 
		
		return(variables->FormatOperands(contextualRightFilter, GetLeftSide().GetString() + " " +
					GetCompareOperator().GetString() + " "));
	}
}

		
ParsedParameter::ParsedParameter(const Variables &variables, std::string token)
	: token(token), type(Invalid)
{
	String::Trim(token);

	if(token.empty())
		return;

	// if the only token is a number FixedInteger/Byte/Adress must be specified
	if(String::FirstCharLikeNumber(token, "&H")) { // a number
		if(!String::ToInt(token, numLiteralValue, "&H"))
			return;
		type = NumLiteral;
	} else if(token.front() == '"' && token.back() == '"' && token.size() >= 2) { // if it starts and ends like a string
		type = StringLiteral;
		strLiteralValue = token.substr(1, token.size() - 2);
	} else { // check if it is a valid variable name
		variable = variables.GetOperand(token);
		type = VariableName;
	}
}

bool ParsedParameter::IsValid(int filter) const
{
	switch(type) {
	case NumLiteral:
		return(IsValidNumLiteral(filter));
	case StringLiteral:
		return(IsValidStringLiteral(filter));
	case VariableName:
		return(IsValidVariable(filter));
	default:
		return(false);
	}
}

bool ParsedParameter::IsValidNumLiteral(int filter) const
{
	if(numLiteralValue >= FixedIntegerMin && numLiteralValue <= FixedIntegerMax && (filter & FixedInteger)) {
		return(true);
	} else if(numLiteralValue >= FixedAddressMin && numLiteralValue <= FixedAddressMax && (filter & FixedAddress)) {
		return(true);
	} else {
		/*if(filter & (FixedByte | FixedInteger | FixedAddress))
			errorMessage = TR_YOU_ENTERED_A_NUMBER_BUT_THIS_NUMBER_IS_TO_BIG_OR_NEGATIVE_THIS_IS_NOW_ALLOWED_HERE;
		else
			errorMessage = TR_YOU_ENTERED_A_NUMBER_BUT_YOU_MAY_NOT_ENTER_A_NUMBER_HERE;*/
		return(false);
	}
}

bool ParsedParameter::IsValidStringLiteral(int filter) const
{
	/*if(!(filter & FixedString)) {
		//errorMessage = TR_YOU_STARTED_A_STRING_WITH_AN_QUOTATION_MARK_BUT_YOU_MAY_NOT_DO_THIS_HERE;
		return(false);
	}

	string::const_iterator secondQuote = find(strLiteralValue.begin() + 1, strLiteralValue.end(), '"');

	if(secondQuote == strLiteralValue.end() - 1) {
		return(true);
	} else {
		//errorMessage = TR_EITHER_A_SECOND_QUOTATION_MARK_IS_MISSING_OR_THERE_ARE_TOO_MUCH_OF_THEM;
		return(false);
	}*/
	return(true);
}

bool ParsedParameter::IsValidVariable(int filter) const
{
	if(variable != NULL && (variable->GetType() & filter)) {
		return(true);
	} else {
		//errorMessage = TR_THIS_VARIABLE_DOES_NOT_EXISTS_HERE_HAVE_YOU_TYPED_IT_WRONG;
		return(false);
	}
}

const string &ParsedParameter::GetText() const
{
	return(token);
}
