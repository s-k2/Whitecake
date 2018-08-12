#ifndef COMMENT_H
#define COMMENT_H

#include "ChartItem.h"

#include "Platform.h"

class Comment : public ChartItem
{
public:
	Comment(Sub *sub, int x, int y);
	explicit Comment(Sub *sub); // don't create dependend items

	virtual bool HasConnectionPoint();
	virtual bool WantsDrag(int x, int y);

	virtual void OnPaint(Drawing *drawing, int flags);
	virtual bool IsInItem(int x, int y);
	virtual bool IsInItem(int x, int y, int width, int height);
	virtual bool OnEdit(NativeWindow parent);
	
	void WriteXML(XMLWriter *xml);
	void ReadXML(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *idRefs);

	const std::string &GetComment() const
		{ return(comment); };
	void SetComment(const std::string &newComment)
		{ comment = newComment; };
	
	static const int Width, Height;
private:
	std::string comment;
};

class EditCommentDlg : public NativeDialog
{
public:
	EditCommentDlg(NativeWindow parent, Comment *comment);
	~EditCommentDlg();

	virtual void PutControls();
	virtual bool OnOK();

private:
	Comment *comment;
	NativeButton *cancelButton;
	NativeButton *okButton;
	NativeEdit *edit;
};


#endif /* COMMENT_H */
