#ifndef VARIABLES_H
#define VARIABLES_H

#include <string>
#include <vector>

class XMLReader;
class XMLWriter;

#define NoArg 0
#define NoRet 0
#define IntegerVariable 1
#define ByteVariable 2
#define CharVariable 4
#define FixedString 8
#define FixedInteger 16 // range -2^16 - 2^16
#define FixedByte 32 // range -2^8 - 2^8
#define FixedBit 64 // values: 0 or 1
#define FixedAddress 128
#define IOPort 1024
#define IOPin 2048
#define ADCChannel 4096

#define FixedIntegerMin (-32768)
#define FixedIntegerMax 32767
#define FixedByteMin 0
#define FixedByteMax 255
#define FixedAddressMin 0
#define FixedAddressMax 65535

class Variable
{
public:
	Variable(const std::string &name, int type);
	Variable(const std::string &name, Variable *target);
	
	inline const std::string &GetName() const
		{ return(name); };
	inline int GetType() const
		{ return(type); };

	inline bool IsAlias() const
		{ return(target != NULL); };
	inline Variable *GetTarget() const
		{ return(target); };

	inline void SetName(const std::string &newName)
		{ name = newName; };
	inline void SetTarget(Variable *newTarget)
		{ target = newTarget; };

private:
	std::string name;
	int type;
	Variable *target;
};

typedef Variable *VariableId;
class Variables
{
public:
	Variables();
	~Variables();

	void Read(XMLReader *xml);
	void Write(XMLWriter *xml);

	bool AddAlias(const std::string &name, const std::string &destination);
	bool UpdateAlias(const std::string &oldName, const std::string &newName, const std::string &destination);

	Variable *GetOperand(const std::string &name) const;
	int GetCastableTypes(int destType) const;
	bool IsValidIdentifier(const std::string &name) const;

	std::vector<std::string> FormatOperands(int filter,	
		const std::string &prefix = "", const std::string &postfix = "") const;

	 // if alias is NULL all Variables are possible, else only those of the alias' type
	std::vector<std::string> SuggestAliasTargets(const std::string &start, const std::string &oldName = "") const;
	std::vector<std::string> GetAllAliases() const;
	std::string GetAliasDestination(const std::string &name) const;

private:
	std::vector<Variable *> variables;
	bool usingADC;
};

#endif /* VARIABLES_H */
