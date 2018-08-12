#include "IfBlock.h"

#include <string>
using std::string;
#include <vector>
using std::vector;

#include "BlockConnector.h"
#include "CodeWriteException.h"
#include "Helper/Geometry.h"
#include "Helper/XMLReader.h"
#include "Helper/XMLWriter.h"
#include "Microcontroller.h"
#include "Translate.h"
#include "Project.h"
#include "Sub.h"

const int IfBlock::Width = 160;
const int IfBlock::Height = 96;

IfBlock::IfBlock(Sub *sub, int x, int y)
	: ProgramBlock(sub, x, y, Width, Height)
{
	dependentItems[0] = new BlockConnector(sub, x + Width / 2, y + Height); // no-side
	dependentItems[1] = new BlockConnector(sub, x + Width, y + Height / 2, true); // yes-side
}

IfBlock::IfBlock(Sub *sub)
	: ProgramBlock(sub, Width, Height)
{
	dependentItems[0] = dependentItems[1] = NULL;
}

IfBlock::~IfBlock(void)
{
}

void IfBlock::OnPaint(Drawing *drawing, int flags)
{	
	drawing->FillRhombus(0, 0, Width, Height, 
		flags & PaintNotSelected ? Stock::LighterGrey : Stock::Crimson);

	string text = comp.GetString();
	if(text.empty())
		text = "?";

	drawing->PrintText(15, 0, Width - 30, Height, text, Stock::GuiFont,  
		flags & PaintNotSelected ? Stock::DarkGrey : Stock::White);
	drawing->PrintText(Width - 5, Height / 2 - 18, 50, 18, TR_NO, Stock::GuiFont, 
		flags & PaintNotSelected ? Stock::DarkGrey : Stock::Black, Drawing::LeftTop);
	drawing->PrintText(Width / 2 + 5, Height + 5, 50, 18, TR_YES, Stock::GuiFont, 
		flags & PaintNotSelected ? Stock::DarkGrey : Stock::Black, Drawing::LeftTop);

	if(flags & PaintErrorMark)
		drawing->DrawEllipse(-20, -20, Width + 40, Height + 40, Stock::ThickerCrimsonPen);
}

bool IfBlock::IsInItem(int x, int y)
{
	return(Rhombus::IntersectsWithPoint<Width, Height>(x - GetX(), y - GetY()));
}

bool IfBlock::IsInItem(int x, int y, int width, int height)
{
	return(Rhombus::IntersectWithRectangle<Width, Height>
		(x, y, width, height, CanvasItemGetX(&item), CanvasItemGetY(&item)));
}

bool IfBlock::OnEdit(NativeWindow parent)
{
	EditIfDlg dlg(parent, this, GetSub()->GetProject()->GetMicrocontroller());
	return(dlg.WasOK());
}

void IfBlock::WriteXML(XMLWriter *xml)
{
	xml->OpenTag("IfBlock");
	ChartItem::WriteXML(xml);
	xml->TextTag("Compare", comp.GetString());
	xml->CloseTag("IfBlock");
}

void IfBlock::ReadXML(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *idRefs)
{
	xml->OpenTag("IfBlock");
	ChartItem::ReadXML(xml, subRefs, idRefs);

	std::string compareStr;
	xml->TextTag("Compare", &compareStr);
	comp = ParsedCompare(GetProject()->GetMicrocontroller()->GetVariables(), compareStr);
	xml->CloseTag("IfBlock");
}

void IfBlock::WriteCode(Compiler::Program &program)
{
	BlockConnector *yesConnector = (BlockConnector *) dependentItems[0];
	BlockConnector *noConnector = (BlockConnector *) dependentItems[1];

	if(yesConnector->GetEndItem() == NULL)
		throw CodeWriteException(yesConnector, TR_YES_SIDE_NOT_CONNECTED_WITH_A_BLOCK);
	if(noConnector->GetEndItem() == NULL)
		throw CodeWriteException(noConnector, TR_NO_SIDE_NOT_CONNECTED_WITH_A_BLOCK);
	if(!comp.IsValid(GetProject()->GetMicrocontroller()))
		throw CodeWriteException(this, TR_NO_CONDITION_IN_IFBLOCK);

	std::string noLocationLabel = program.GenerateSymbolName();
	comp.WriteCode(program, noLocationLabel);

	// just print the yes side as it follows the compare
	// it is always ended by an return or an infinite jump, 
	// (as CodeWrite runs 'til the end of the program)
	// so we don't have to care about jumping to 
	// code after this IfBlock
	yesConnector->WriteCode(program);

	program.LabelCurrentLocation(noLocationLabel);
	noConnector->WriteCode(program);
}


EditIfDlg::EditIfDlg(NativeWindow parent, 
	IfBlock *ifBlock, Microcontroller *microcontroller)
{
	this->ifBlock = ifBlock;
	this->microcontroller = microcontroller;

	Create(parent, 470, 364, TR_IF_BLOCK);
}

EditIfDlg::~EditIfDlg()
{
	delete explanation;
	delete okButton;
	delete cancelButton;
	delete suggest;
}

void EditIfDlg::PutControls()
{
	explanation = new NativeLabel(this, 14, 10, 450, 80, TR_IF_EXPLANATION);
	suggest = new NativeSuggest(this, 14, 100, 450, 21, 200);
	suggest->onShowSuggestions.AddHandler(this, (NativeEventHandler) &EditIfDlg::OnSuggest);
	suggest->SetText(ifBlock->GetCompare().GetString());

	okButton = new NativeButton(this, 294, 330, 80, 24, TR_OK, NativeButton::OKButton);
	cancelButton = new NativeButton(this, 384, 330, 80, 24, TR_CANCEL, NativeButton::CancelButton);
}

bool EditIfDlg::OnOK()
{
	ParsedCompare compare(microcontroller->GetVariables(), suggest->GetText());

	if(compare.IsValid(microcontroller)) {
		ifBlock->SetCompare(compare);

		return(true); // valid value entered
	} else {
		return(false);
	}
}

void EditIfDlg::OnSuggest(NativeControl *sender)
{
	ParsedCompare compare(microcontroller->GetVariables(), suggest->GetText());

	suggest->SetSuggestions(compare.SuggestCompletion());
}

