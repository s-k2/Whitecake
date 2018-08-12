#include "Comment.h"

#include "Helper/XMLReader.h"
#include "Helper/XMLWriter.h"
#include "Translate.h"

const int Comment::Width = 128;
const int Comment::Height = 80;

Comment::Comment(Sub *sub, int x, int y)
	: ChartItem(sub, x, y, Width, Height)
{
	isDependent = false;
	dependentItems[0] = dependentItems[1] = NULL;
	comment = TR_COMMENT;
}

Comment::Comment(Sub *sub)
	: ChartItem(sub, Width, Height)
{
	isDependent = false;
	dependentItems[0] = dependentItems[1] = NULL;
	comment = TR_COMMENT;
}

void Comment::OnPaint(Drawing *drawing, int flags)
{
	drawing->FillRect(0, 0, GetWidth(), GetHeight(), 
		flags & PaintNotSelected ? Stock::LighterGrey : Stock::Green);

	drawing->PrintText(2, 2, GetWidth() - 4, GetHeight() - 4, comment, Stock::GuiFont,  
		flags & PaintNotSelected ? Stock::DarkGrey : Stock::White);
}

bool Comment::IsInItem(int x, int y)
{
	return(true);
}

bool Comment::IsInItem(int x, int y, int width, int height)
{
	return(true);
}

bool Comment::HasConnectionPoint()
{
	return(false);
}

bool Comment::OnEdit(NativeWindow parent)
{
	EditCommentDlg dlg(parent, this);
	return(dlg.WasOK());
}

void Comment::WriteXML(XMLWriter *xml)
{
	xml->OpenTag("Comment");
	ChartItem::WriteXML(xml);
	xml->TextTag("comment", comment);
	xml->CloseTag("Comment");
}

void Comment::ReadXML(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *idRefs)
{
	xml->OpenTag("Comment");
	ChartItem::ReadXML(xml, subRefs, idRefs);
	xml->TextTag("comment", &comment);
	xml->CloseTag("Comment");
}

bool Comment::WantsDrag(int x, int y)
{
	return(false);
}

EditCommentDlg::EditCommentDlg(NativeWindow parent, Comment *comment)
{
	this->comment = comment;

	Create(parent, 300, 324, TR_EDIT_COMMENT);
}

EditCommentDlg::~EditCommentDlg()
{
	delete cancelButton;
	delete okButton;
	delete edit;
}

void EditCommentDlg::PutControls()
{
	edit = new NativeEdit(this, 10, 10, 280, 270, comment->GetComment(), true);
	
	okButton = new NativeButton(this, 80, 290, 100, 24, TR_OK, NativeButton::OKButton);
	cancelButton = new NativeButton(this, 190, 290, 100, 24, TR_CANCEL, NativeButton::CancelButton);
}

bool EditCommentDlg::OnOK()
{
	comment->SetComment(edit->GetText());

	return(true);
}
