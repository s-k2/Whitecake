#ifndef PROGRAM_FLOW_H
#define PROGRAM_FLOW_H

#include "ProgramBlock.h"

#include "Platform.h"

class StartBlock : public ProgramBlock
{
public:
	StartBlock(Sub *sub, int x, int y);
	explicit StartBlock(Sub *sub); // don't create dependend items

	virtual bool HasConnectionPoint();

	virtual void OnPaint(Drawing *drawing, int flags);
	virtual bool IsInItem(int x, int y);
	virtual bool IsInItem(int x, int y, int width, int height);
	
	void WriteXML(XMLWriter *xml);
	void ReadXML(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *idRefs);
	void WriteCode(Compiler::Program &program);

	static const int Width, Height, CornerRadius;
};

class EndBlock : public ProgramBlock
{
public:
	EndBlock(Sub *sub, int x, int y);
	explicit EndBlock(Sub *sub); // don't create dependend items

	virtual bool HasConnectionPoint();

	virtual void OnPaint(Drawing *drawing, int flags);
	virtual bool IsInItem(int x, int y);
	virtual bool IsInItem(int x, int y, int width, int height);
	
	void WriteXML(XMLWriter *xml);
	void ReadXML(XMLReader *reader, 
		SubReferenceVector *subRefs, IdItemMap *idRefs);
	void WriteCode(Compiler::Program &program);

	static const int Width, Height, CornerRadius;
};

class CallSub : public ProgramBlock
{
public:
	CallSub(Sub *sub, int x, int y);
	explicit CallSub(Sub *sub); // don't create dependend items

	virtual void OnPaint(Drawing *drawing, int flags);
	virtual bool IsInItem(int x, int y);
	virtual bool IsInItem(int x, int y, int width, int height);
	virtual bool OnEdit(NativeWindow parent);
	
	void WriteXML(XMLWriter *xml);
	void ReadXML(XMLReader *reader, 
		SubReferenceVector *subRefs, IdItemMap *idRefs);
	void WriteCode(Compiler::Program &program);

	void SetCalledSub(Sub *sub) 
		{ this->calledSub = sub; };
	Sub *GetCalledSub()
		{ return(calledSub); };

	static const int Width, Height;

private:
	Sub *calledSub;
};

#endif /* PROGRAM_FLOW_H */
