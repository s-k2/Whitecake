#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include <string>
#include <vector>

#include "ProgramBlock.h"
#include "Platform.h"
#include "Microcontroller.h"
#include "Expressions.h"

class Assignment : public ProgramBlock
{
public:
	Assignment(Sub *sub, int x, int y);
	explicit Assignment(Sub *sub); // don't create dependend items

	virtual void OnPaint(Drawing *drawing, int flags);
	virtual bool IsInItem(int x, int y);
	virtual bool IsInItem(int x, int y, int width, int height);
	virtual bool OnEdit(NativeWindow parent);
	
	void WriteXML(XMLWriter *xml);
	void ReadXML(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *idRefs);
	void WriteCode(Compiler::Program &program);
	inline const ParsedAssignment &GetAssignment()
		{ return(assignment); };
	inline void SetAssignment(const ParsedAssignment &assignment)
		{ this->assignment = assignment; };

	static const int Width;
	static const int Height;
private:

	ParsedAssignment assignment;
};

class EditAssignmentDlg : public NativeDialog
{
public:
	EditAssignmentDlg(NativeWindow parent, Assignment *assignment, 
	Microcontroller *microcontroller);
	~EditAssignmentDlg();

	virtual void PutControls();
	virtual bool OnOK();

private:
	Assignment *assignment;
	Microcontroller *microcontroller;

	NativeLabel *explanation;
	NativeSuggest *suggest;
	NativeButton *okButton;
	NativeButton *cancelButton;

	void OnSuggest(NativeControl *sender);
};

#endif /* ASSIGNMENT_H */
