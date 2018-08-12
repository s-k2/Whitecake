#ifndef IFBLOCK_H
#define IFBLOCK_H

#include <string>
#include <vector>

#include "ProgramBlock.h"
#include "Microcontroller.h"
#include "Expressions.h"

class IfBlock : public ProgramBlock
{
public:
	IfBlock(Sub *sub, int x, int y);
	explicit IfBlock(Sub *sub); // don't create dependend items
	virtual ~IfBlock(void);

	virtual void OnPaint(Drawing *drawing, int flags);
	virtual bool IsInItem(int x, int y);
	virtual bool IsInItem(int x, int y, int width, int height);
	virtual bool OnEdit(NativeWindow parent);
		
	void WriteXML(XMLWriter *xml);
	void ReadXML(XMLReader *reader, 
		SubReferenceVector *subRefs, IdItemMap *idRefs);
	void WriteCode(Compiler::Program &program);

	static const int Width;
	static const int Height;

	inline void SetCompare(const ParsedCompare &compare)
		{ this->comp = compare; };
	inline const ParsedCompare &GetCompare() const
		{ return(comp); };

private:
	ParsedCompare comp;
};



class EditIfDlg : public NativeDialog
{
public:
	EditIfDlg(NativeWindow parent, IfBlock *ifBlock, Microcontroller *microcontroller);
	~EditIfDlg();

	virtual void PutControls();
	virtual bool OnOK();

private:
	IfBlock *ifBlock;
	Microcontroller *microcontroller;

	NativeLabel *explanation;
	NativeSuggest *suggest;
	NativeButton *okButton;
	NativeButton *cancelButton;

	void OnSuggest(NativeControl *sender);
};

#endif /* IFBLOCK_H */
