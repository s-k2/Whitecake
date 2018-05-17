#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>

#include "ProgramBlock.h"
#include "Platform.h"

#define InstructionMaxArgs 3
struct InstructionArg
{
	int type;
	const char *text;
};

struct InstructionInfo
{
	const char *idString;
	const char *text;
	const char *basicCode;

	size_t argsCount;
	struct InstructionArg args[InstructionMaxArgs];

	int ret;
};


class Instruction : public ProgramBlock
{
public:
	Instruction(Sub *sub, int x, int y, InstructionInfo *info);
	explicit Instruction(Sub *sub); // don't create dependend items
	virtual ~Instruction(void);

	virtual void OnPaint(Drawing *drawing, int flags);
	virtual bool IsInItem(int x, int y);
	virtual bool IsInItem(int x, int y, int width, int height);
	virtual bool OnEdit(NativeWindow parent);

	static InstructionInfo InstructionInfos[];
	static const size_t InstructionInfosCount;
	inline InstructionInfo *GetInstructionInfo()
		{ return(info); };

	inline void SetArg(int index, std::string value)
		{ args[index] = value; };
	inline const std::string &GetArg(int index)
		{ return(args[index]); };
	inline void SetResult(std::string result)
		{ this->result = result; };
	inline const std::string &GetResult()
		{ return(result); };
	
	void WriteXML(XMLWriter *xml);
	void ReadXML(XMLReader *reader, 
		SubReferenceVector *subRefs, IdItemMap *idRefs);
	void WriteBasic(BasicWriter *basic);
	void WriteCode(Compiler::Program &program);

	static const int Width, Height, ShiftValue;
	static const double Shift;
	//static const double Shift; Must be a macro for compile-time evaluation

private:
	InstructionInfo *info;
	std::string args[InstructionMaxArgs];
	std::string result;
};

class Microcontroller;

class EditInstructionDlg : public NativeDialog
{
public:
	EditInstructionDlg(NativeWindow parent, Instruction *instruction, Microcontroller *microcontroller);
	~EditInstructionDlg();

	virtual void PutControls();
	virtual bool OnOK();

	void OnListboxDblClick(NativeControl *sender);

private:
	Instruction *instruction;
	struct InstructionInfo *info;
	Microcontroller *microcontroller;

	NativeLabel *labels[InstructionMaxArgs + 1]; // +1 due to the return-field
	NativeEdit *editFields[InstructionMaxArgs + 1]; // +1 due to the return-field
	NativeListbox *listboxes[InstructionMaxArgs + 1]; // +1 due to the return-field
	NativeButton *okButton;

};

class SelectInstructionDlg : public NativeDialog
{
public:
	explicit SelectInstructionDlg(NativeWindow parent);
	~SelectInstructionDlg();

	virtual void PutControls();
	virtual bool OnOK();

	void OnSelectionChange(NativeControl *sender);

	inline InstructionInfo *GetSelectedInfo()
		{ return(selectedInstruction); };

private:
	NativeListbox *listbox;

	InstructionInfo *selectedInstruction;
};

#endif /* INSTRUCTION_H */
