#include "ProgramFlow.h"

#include <string>
using std::string;
#include <utility>
using std::pair;

#include "Sub.h"
#include "BlockConnector.h"
#include "Helper/BasicWriter.h"
#include "Helper/XMLReader.h"
#include "Helper/XMLWriter.h"
#include "Compiler.h"
#include "Project.h"
#include "Translate.h"

const int StartBlock::Width = 128;
const int StartBlock::Height = 72;
const int StartBlock::CornerRadius = 18;

StartBlock::StartBlock(Sub *sub, int x, int y)
	: ProgramBlock(sub, x, y, Width, Height)
{
	dependentItems[0] = new BlockConnector(sub, x + Width / 2, y + Height);
	dependentItems[1] = NULL;
}

StartBlock::StartBlock(Sub *sub)
	: ProgramBlock(sub, Width, Height)
{
	dependentItems[0] = dependentItems[1] = NULL;
}

void StartBlock::OnPaint(Drawing *drawing, int flags)
{
	drawing->FillRoundedRect(0, 0, Width, Height, CornerRadius, 
		flags & PaintNotSelected ? Stock::LighterGrey : Stock::Green);

	drawing->PrintText(0, 0, Width, Height, TR_START, Stock::GuiFont,  
		flags & PaintNotSelected ? Stock::DarkGrey : Stock::White);
}

bool StartBlock::IsInItem(int x, int y)
{
	return(true);
}

bool StartBlock::IsInItem(int x, int y, int width, int height)
{
	return(true);
}

bool StartBlock::HasConnectionPoint()
{
	// the start point have no input connectors, because... it's the start ;-)
	return(false);
}

void StartBlock::WriteXML(XMLWriter *xml)
{
	xml->OpenTag("StartBlock");
	ChartItem::WriteXML(xml);
	xml->CloseTag("StartBlock");
}

void StartBlock::ReadXML(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *idRefs)
{
	xml->OpenTag("StartBlock");
	ChartItem::ReadXML(xml, subRefs, idRefs);
	xml->CloseTag("StartBlock");

	// if no script kiddie altered the xml-file there is only one start-block!
	// but even if not, it's not that problem, but it should be funny to see
	// two start blocks in the chart (and just one beeing the entry-point)
	GetSub()->SetStartBlock(this);
}

void StartBlock::WriteBasic(BasicWriter *basic)
{
	basic->PutCode("' Start here");

	// continue with the dependend items
	ChartItem::WriteBasic(basic);
}

void StartBlock::WriteCode(Compiler::Program &program)
{
	program.LabelCurrentLocation(GetSub()->GetName());
	ChartItem::WriteCode(program);
}

const int EndBlock::Width = 128;
const int EndBlock::Height = 72;
const int EndBlock::CornerRadius = 18;

EndBlock::EndBlock(Sub *sub, int x, int y)
	: ProgramBlock(sub, x, y, Width, Height)
{
	dependentItems[0] = NULL; // it makes no sense to connect anything here ;-)
	dependentItems[1] = NULL;
}

EndBlock::EndBlock(Sub *sub)
	: ProgramBlock(sub, Width, Height)
{
	dependentItems[0] = dependentItems[1] = NULL;
}

void EndBlock::OnPaint(Drawing *drawing, int flags)
{
	drawing->FillRoundedRect(0, 0, Width, Height, CornerRadius, 
		flags & PaintNotSelected ? Stock::LighterGrey : Stock::Crimson);

	drawing->PrintText(0, 0, Width, Height, TR_END, Stock::GuiFont,  
		flags & PaintNotSelected ? Stock::DarkGrey : Stock::White);
	
	if(flags & PaintErrorMark)
		drawing->DrawEllipse(-20, -20, Width + 40, Height + 40, Stock::ThickerCrimsonPen);

	//if(this->GetInputConnectorCount() == 0) {
	//	drawing->DrawLine(Width / 2 - 5, 0 - 5, Width / 2 + 5, 0 + 5,
	//		flags & PaintNotSelected ? Stock::ThickerLighterGreyPen : Stock::ThickerBlackPen);
	//	drawing->DrawLine(Width / 2 - 5, 0 + 5, Width / 2 + 5, 0 - 5, 
	//		flags & PaintNotSelected ? Stock::ThickerLighterGreyPen : Stock::ThickerBlackPen);
	//}
}

bool EndBlock::IsInItem(int x, int y)
{
	return(true);
}

bool EndBlock::IsInItem(int x, int y, int width, int height)
{
	return(true);
}

bool EndBlock::HasConnectionPoint()
{
	// other items can connect here
	return(true);
}

void EndBlock::WriteXML(XMLWriter *xml)
{
	xml->OpenTag("EndBlock");
	ChartItem::WriteXML(xml);
	xml->CloseTag("EndBlock");
}

void EndBlock::ReadXML(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *idRefs)
{
	xml->OpenTag("EndBlock");
	ChartItem::ReadXML(xml, subRefs, idRefs);
	xml->CloseTag("EndBlock");
}

void EndBlock::WriteBasic(BasicWriter *basic)
{
	basic->PutCode("Exit Sub");
}

void EndBlock::WriteCode(Compiler::Program &program)
{
	program.ReturnFunction();
}

const int CallSub::Width = 128;
const int CallSub::Height = 72;

CallSub::CallSub(Sub *sub, int x, int y)
	: ProgramBlock(sub, x, y, Width, Height)
{
	dependentItems[0] = new BlockConnector(sub, x + Width / 2, y + Height);
	dependentItems[1] = NULL;

	calledSub = NULL;
}

CallSub::CallSub(Sub *sub)
	: ProgramBlock(sub, Width, Height)
{
	dependentItems[0] = dependentItems[1] = NULL;

	calledSub = NULL;
}

void CallSub::OnPaint(Drawing *drawing, int flags)
{
	drawing->FillRect(0, 0, Width, Height, 
		flags & PaintNotSelected ? Stock::LighterGrey : Stock::SteelBlue);
	drawing->FillRect(0, 0, Width * 0.1, Height, Stock::TransparentWhite);
	drawing->FillRect(Width * 0.9, 0, 
		Width * 0.1, Height, Stock::TransparentWhite);
	
	drawing->PrintText(0, 0, Width, Height / 2, TR_CALL, Stock::GuiFont,  
		flags & PaintNotSelected ? Stock::DarkGrey : Stock::White);
	drawing->PrintText(0, Height / 2, Width, Height / 2, 
		GetCalledSub() != NULL ? GetCalledSub()->GetName() : "?", 
		Stock::GuiFont,  flags & PaintNotSelected ? Stock::DarkGrey : Stock::White);
	
	if(flags & PaintErrorMark)
		drawing->DrawEllipse(-20, -20, Width + 40, Height + 40, Stock::ThickerCrimsonPen);

}

bool CallSub::IsInItem(int x, int y)
{
	return(true);
}

bool CallSub::IsInItem(int x, int y, int width, int height)
{
	return(true);
}

bool CallSub::OnEdit(NativeWindow parent)
{
	EditCallSubDlg dlg(parent, this);
	return(dlg.WasOK());
}

void CallSub::WriteXML(XMLWriter *xml)
{
	xml->OpenTag("CallSub");
	ChartItem::WriteXML(xml);
	string subName;
	xml->TextTag("Sub", calledSub != NULL ? calledSub->GetName() : "");
	xml->CloseTag("CallSub");
}

void CallSub::ReadXML(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *idRefs)
{
	xml->OpenTag("CallSub");
	ChartItem::ReadXML(xml, subRefs, idRefs);
	string subName;
	xml->TextTag("Sub", &subName);
	subRefs->push_back(pair<Sub **, string>(&calledSub, subName));
	calledSub = NULL;
	xml->CloseTag("CallSub");
}

void CallSub::WriteBasic(BasicWriter *basic)
{
	if(GetCalledSub() == NULL)
		throw WriteBasicException(this, TR_MISSING_SUBNAME_IN_CALL);

	basic->CallSub(Sub::FunctionPrefix + GetCalledSub()->GetName());
	
	// continue with the dependend items
	ChartItem::WriteBasic(basic);
}

void CallSub::WriteCode(Compiler::Program &program)
{
	if(GetCalledSub() == NULL)
		throw WriteBasicException(this, TR_MISSING_SUBNAME_IN_CALL);

	program.Call(Compiler::UnknownAddress(calledSub->GetName()));
	ChartItem::WriteCode(program);
}

EditCallSubDlg::EditCallSubDlg(NativeWindow parent, CallSub *callSub)
{
	this->callSub = callSub;

	Create(parent, 300, 404, TR_CALL_SUB);
}

EditCallSubDlg::~EditCallSubDlg()
{
	delete explanation;
	delete cancelButton;
	delete okButton;
	delete listbox;
}

void EditCallSubDlg::PutControls()
{
	explanation = new NativeLabel(this, 14, 10, 280, 70, TR_CALL_SUB_EXPLANATION);
	listbox = new NativeListbox(this, 14, 90, 280, 270);
	listbox->onSelectionDoubleClick.AddHandler(this, (NativeEventHandler) &EditCallSubDlg::OnListboxDblClick);

	Project *project = callSub->GetProject();
	for(size_t i = 0; i < project->GetChartsCount(); i++) {
		listbox->AddItem(project->GetChart(i)->GetName(), project->GetChart(i));
	}
	
	okButton = new NativeButton(this, 84, 370, 100, 24, TR_OK, NativeButton::OKButton);
	cancelButton = new NativeButton(this, 194, 370, 100, 24, TR_CANCEL, NativeButton::CancelButton);
}

bool EditCallSubDlg::OnOK()
{
	if(listbox->GetSelectedIndex() >= 0) {
		callSub->SetCalledSub((Sub *) listbox->GetUserData(listbox->GetSelectedIndex()));
		return(true);
	}

	return(false);
}

void EditCallSubDlg::OnListboxDblClick(NativeControl *sender)
{
	if(OnOK())
		FinishDialogWithOK();
}
