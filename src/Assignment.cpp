#include "Assignment.h"

#include <string>
using std::string;
#include <vector>
using std::vector;

#include "BlockConnector.h"
#include "Helper/BasicWriter.h"
#include "Helper/XMLReader.h"
#include "Helper/XMLWriter.h"
#include "Microcontroller.h"
#include "Project.h"
#include "Sub.h"
#include "Translate.h"

const int Assignment::Width = 128;//144;
const int Assignment::Height = 72;//80;

Assignment::Assignment(Sub *sub, int x, int y)
	: ProgramBlock(sub, x, y, Width, Height)
{
	dependentItems[0] = new BlockConnector(sub, x + Width / 2, y + Height);
	dependentItems[1] = NULL;
}

Assignment::Assignment(Sub *sub)
	: ProgramBlock(sub, Width, Height)
{
	dependentItems[0] = dependentItems[1] = NULL;
}

void Assignment::OnPaint(Drawing *drawing, int flags)
{
	drawing->FillRect(0, 0, Width, Height, 
		flags & PaintNotSelected ? Stock::LighterGrey : Stock::SteelBlue);
	
	string text = assignment.GetString("\n");
	if(text.empty())
		text = "?";
	drawing->PrintText(0, 0, Width, Height, text, Stock::GuiFont, 
		flags & PaintNotSelected ? Stock::DarkGrey : Stock::White);

	if(flags & PaintErrorMark)
		drawing->DrawEllipse(-20, -20, Width + 40, Height + 40, Stock::ThickerCrimsonPen);
}

bool Assignment::IsInItem(int x, int y)
{
	return(true);
}

bool Assignment::IsInItem(int x, int y, int width, int height)
{
	return(true);
}

bool Assignment::OnEdit(NativeWindow parent)
{
	EditAssignmentDlg dlg(parent, this, GetProject()->GetMicrocontroller());
	return(dlg.WasOK());
}
	
void Assignment::WriteXML(XMLWriter *xml)
{
	xml->OpenTag("Assignment");
	ChartItem::WriteXML(xml);
	xml->TextTag("Assignment", assignment.GetString());
	xml->CloseTag("Assignment");
}

void Assignment::ReadXML(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *idRefs)
{
	xml->OpenTag("Assignment");
	ChartItem::ReadXML(xml, subRefs, idRefs);
	
	string assignmentStr;
	xml->TextTag("Assignment", &assignmentStr);
	assignment = ParsedAssignment(GetProject()->GetMicrocontroller()->GetVariables(), assignmentStr);
	
	xml->CloseTag("Assignment");
}

void Assignment::WriteBasic(BasicWriter *basic)
{
	if(!assignment.IsValid(GetProject()->GetMicrocontroller()))
		throw WriteBasicException(this, TR_NO_FORMULA_IN_ASSIGNMENT);

	basic->PutCode(assignment.GetString());

	// continue with the dependend items
	ChartItem::WriteBasic(basic);
}

void Assignment::WriteCode(Compiler::Program &program)
{
	if(!assignment.IsValid(GetProject()->GetMicrocontroller()))
		throw WriteBasicException(this, TR_NO_FORMULA_IN_ASSIGNMENT);

	assignment.WriteCode(program);

	// continue with the dependend items
	ChartItem::WriteCode(program);
}


EditAssignmentDlg::EditAssignmentDlg(NativeWindow parent, 
	Assignment *assignment, Microcontroller *microcontroller)
{
	this->assignment = assignment;
	this->microcontroller = microcontroller;

	Create(parent, 470, 364, TR_ASSIGNMENT);
}

EditAssignmentDlg::~EditAssignmentDlg()
{
	delete explanation;
	delete okButton;
	delete cancelButton;
	delete suggest;
}

void EditAssignmentDlg::PutControls()
{
	explanation = new NativeLabel(this, 14, 10, 450, 80, TR_ASSIGNMENT_EXPLANATION);
	suggest = new NativeSuggest(this, 14, 100, 450, 21, 200);
	suggest->onShowSuggestions.AddHandler(this, (NativeEventHandler) &EditAssignmentDlg::OnSuggest);
	suggest->SetText(assignment->GetAssignment().GetString());

	okButton = new NativeButton(this, 294, 330, 80, 24, TR_OK, NativeButton::OKButton);
	cancelButton = new NativeButton(this, 384, 330, 80, 24, TR_CANCEL, NativeButton::CancelButton);
}

bool EditAssignmentDlg::OnOK()
{
	string text = suggest->GetText();

	ParsedAssignment parsedAssignment(microcontroller->GetVariables(), text);

	if(parsedAssignment.IsValid(microcontroller)) {
		assignment->SetAssignment(parsedAssignment);

		return(true); // valid value entered
	} else {
		return(false);
	}
}

void EditAssignmentDlg::OnSuggest(NativeControl *sender)
{
	ParsedAssignment parsedAssignment(microcontroller->GetVariables(), suggest->GetText());
	
	suggest->SetSuggestions(parsedAssignment.SuggestCompletions());
}
