#include "Variables.h"

using std::string;
using std::vector;

#include "Helper/XMLReader.h"
#include "Helper/XMLWriter.h"
#include "Settings.h"

Variable::Variable(const string &name, int type)
{
	this->name = name;
	this->type = type;
	this->target = NULL;
}

Variable::Variable(const string &name, Variable *target)
{
	this->name = name;
	this->type = target->GetType();
	// get real alias (in case of aliases to aliases)
	while(target->target != NULL) // TODO: Prevent endless loops here!!!
		target = target->target;
	this->target = target;
}

Variables::Variables()
{
	for(size_t i = 0; i < theSettings.GetIntegerVariables().size(); i++)
		variables.push_back(new Variable(theSettings.GetIntegerVariables()[i], IntegerVariable));
	for(size_t i = 0; i < theSettings.GetIOPorts().size(); i++)
		variables.push_back(new Variable(theSettings.GetIOPorts()[i], IOPort));
	for(size_t i = 0; i < theSettings.GetIOPins().size(); i++)
		variables.push_back(new Variable(theSettings.GetIOPins()[i], IOPin));
	for(size_t i = 0; i < theSettings.GetADCChannels().size(); i++)
		variables.push_back(new Variable(theSettings.GetADCChannels()[i], ADCChannel));
	usingADC = !theSettings.GetADCChannels().empty();

	for(size_t i = 0; i < theSettings.GetAliases().size(); i++) {
		Variable *target = GetOperand(theSettings.GetAliases()[i].second);
		if(target)
			variables.push_back(new Variable(theSettings.GetAliases()[i].first, target));
	}
}

Variables::~Variables()
{
	for(vector<Variable *>::iterator it = variables.begin(); it != variables.end(); it++)
		delete *it;
}

void Variables::Read(XMLReader *xml)
{
	for(vector<Variable *>::iterator it = variables.begin(); it != variables.end(); it++)
		delete *it;
	variables.clear();

	xml->OpenTag("Integers");
	while(xml->GetCurrentNode()->GetChildCount() > 0) {
		std::string varName;
		xml->TextTag("Integer", &varName);
		variables.push_back(new Variable(varName, IntegerVariable));
	}
	xml->CloseTag("Integers");
	xml->OpenTag("Ports");
	while(xml->GetCurrentNode()->GetChildCount() > 0) {
		std::string varName;
		xml->TextTag("Port", &varName);
		variables.push_back(new Variable(varName, IOPort));
	}
	xml->CloseTag("Ports");
	xml->OpenTag("Pins");
	while(xml->GetCurrentNode()->GetChildCount() > 0) {
		std::string varName;
		xml->TextTag("Pin", &varName);
		variables.push_back(new Variable(varName, IOPin));
	}
	xml->CloseTag("Pins");

	usingADC = false;
	xml->OpenTag("ADCs");
	while(xml->GetCurrentNode()->GetChildCount() > 0) {
		std::string varName;
		xml->TextTag("ADC", &varName);
		variables.push_back(new Variable(varName, ADCChannel));
		usingADC = true;
	}
	xml->CloseTag("ADCs");

	
	xml->OpenTag("Aliases");
	while(xml->GetCurrentNode()->GetChildCount() > 0) {
		xml->OpenTag("Alias");
		std::string name, targetName;
		xml->TextTag("Name", &name);
		xml->TextTag("Target", &targetName);
		Variable *target = GetOperand(targetName);
		if(target)
			variables.push_back(new Variable(name, target));
		xml->CloseTag("Alias");
	}
	xml->CloseTag("Aliases");
}

void Variables::Write(XMLWriter *xml)
{
	xml->OpenTag("Integers");
	for(vector<Variable *>::iterator it = variables.begin(); it != variables.end(); it++) {
		if((*it)->GetType() == IntegerVariable && !(*it)->IsAlias())
			xml->TextTag("Integer", (*it)->GetName());
	}
	xml->CloseTag("Integers");
	xml->OpenTag("Ports");
	for(vector<Variable *>::iterator it = variables.begin(); it != variables.end(); it++) {
		if((*it)->GetType() == IOPort && !(*it)->IsAlias())
			xml->TextTag("Port", (*it)->GetName());
	}
	xml->CloseTag("Ports");
	xml->OpenTag("Pins");
	for(vector<Variable *>::iterator it = variables.begin(); it != variables.end(); it++) {
		if((*it)->GetType() == IOPin && !(*it)->IsAlias())
			xml->TextTag("Pin", (*it)->GetName());
	}
	xml->CloseTag("Pins");
	xml->OpenTag("ADCs");
	for(vector<Variable *>::iterator it = variables.begin(); it != variables.end(); it++) {
		if((*it)->GetType() == ADCChannel && !(*it)->IsAlias())
			xml->TextTag("ADC", (*it)->GetName());
	}
	xml->CloseTag("ADCs");

	xml->OpenTag("Aliases");
	for(vector<Variable *>::iterator it = variables.begin(); it != variables.end(); it++) {
		if((*it)->IsAlias()) {
			xml->OpenTag("Alias");
			xml->TextTag("Name", (*it)->GetName());
			xml->TextTag("Target", (*it)->GetTarget()->GetName());
			xml->CloseTag("Alias");
		}
	}
	xml->CloseTag("Aliases");
}

vector<string> Variables::FormatOperands(int filter, const std::string &prefix, const std::string &postfix) const
{
	vector<string> operands;
	// put aliases first...
	for(auto it = variables.begin(); it != variables.end(); ++it)
		if((filter & (*it)->GetType()) != 0 && (*it)->IsAlias())
			operands.push_back(prefix + (*it)->GetName() + postfix);
	// ...and then all other variables
	for(auto it = variables.begin(); it != variables.end(); ++it)
		if((filter & (*it)->GetType()) != 0 && !(*it)->IsAlias())
			operands.push_back(prefix + (*it)->GetName() + postfix);

	return(operands);
}

Variable *Variables::GetOperand(const string &name) const
{
	for(auto it = variables.begin(); it != variables.end(); it++)
		if((*it)->GetName() == name)
			return(*it);

	return(NULL);
}

int Variables::GetCastableTypes(int destType) const
{
	switch(destType) {
	case IntegerVariable:
		return(IntegerVariable | IOPin | FixedBit | FixedInteger);
	case IOPort:
		return(IOPin | FixedBit);
	case IOPin:
		return(IOPin);
	default:
		return(0);
	}
}

vector<string> Variables::GetAllAliases() const
{
	vector<string> all;
	for(auto it = variables.begin(); it != variables.end(); ++it)
		if((*it)->IsAlias())
			all.push_back((*it)->GetName());

	return(all);
}

string Variables::GetAliasDestination(const std::string &name) const
{
	Variable *alias = GetOperand(name);
	if(alias && alias->IsAlias())
		return(alias->GetTarget()->GetName());
	else
		return("");
}

bool Variables::AddAlias(const string &name, const string &destination)
{
	// does the destination exists?
	Variable *target = GetOperand(destination);
	if(target == NULL)
		return(false);

	// is the new name possible and not already in use?
	if(!IsValidIdentifier(name) || GetOperand(name) != NULL)
		return(false);

	variables.push_back(new Variable(name, target));
	return(true);
}

bool Variables::UpdateAlias(const string &oldName, const string &newName, const string &destination)
{
	Variable *alias = GetOperand(oldName);
	if(alias == NULL)
		return(false);

	// does the destination exists?
	Variable *target = GetOperand(destination);
	if(target == NULL)
		return(false);

	// is the name possible
	if(!IsValidIdentifier(newName))
		return(false);

	// do the types fit?
	if(target->GetType() != alias->GetTarget()->GetType())
		return(false);

	alias->SetName(newName);
	alias->SetTarget(target);
	return(true);
}

bool Variables::IsValidIdentifier(const string &name) const
{
	if(name.empty())
		return(false);

	if(!isalpha(name[0]))
		return(false);

	for(size_t i = 0; i < name.size(); i++)
		if(!isalnum(name[i]) && name[i] != '_')
			return(false);

	return(true);
}

vector<string> Variables::SuggestAliasTargets(const string &start, const string &name) const
{
	vector<string> allTargets;
	int filter = 0;
	Variable *alias = GetOperand(name);
	if(alias != NULL)
		filter = alias->GetType();

	for(auto it = variables.begin(); it != variables.end(); it++) {
		if((filter == 0 || (filter & (*it)->GetType()) != 0) && // if there is no filter or the supplied filter is matched
			!(*it)->IsAlias() && // and this target is not an alias
			(start.empty() || (*it)->GetName().substr(0, start.size()) == start)) // and it starts with the same letters
		{
			allTargets.push_back((*it)->GetName());
		}
	}

	return(allTargets);
}
