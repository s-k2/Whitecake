#include "Sub.h"

#include <string>
using std::string;

#include "Assignment.h"
#include "BlockConnector.h"
#include "ChartView.h"
#include "Comment.h"
#include "Compiler.h"
#include "IfBlock.h"
#include "Instruction.h"
#include "Microcontroller.h"
#include "ProgramBlock.h"
#include "ProgramFlow.h"
#include "Project.h"
#include "Helper/BasicWriter.h"
#include "Helper/String.h"
#include "Helper/XMLWriter.h"
#include "Helper/XMLReader.h"
#include "Translate.h"
#include "Variables.h"

#include <vector>
const char *Sub::FunctionPrefix = "Function";


Sub::Sub(Project *project)
{
	this->project = project;

	CanvasInit(&canvas);
	CanvasSetViewOrigin(GetCanvas(), -96, -96);

	startBlock = new StartBlock(this, 96, 96);

	if(project->GetChartsCount() == 0)
		name = TR_START;
	else
		name = "Funktion" + String::IntToStr(project->GetChartsCount()); // TODO: Translate
}

Sub::Sub(Project *project, XMLReader *reader, 
		SubReferenceVector *subRefs)
{
	this->project = project;

	CanvasInit(&canvas);

	CanvasSetViewOrigin(GetCanvas(), -96, -96);

	ReadXML(reader, subRefs);
}

void Sub::DeleteItem(Canvas *canvas, CanvasItem *item, void *userData)
{
	register ChartItem *chartItem = (ChartItem *) item->itemData;

	// just delete the independend items, all dependend item's will be delteted
	// by its owner...
	if(!chartItem->IsDependent())
		delete chartItem;
}

void Sub::DeleteReferences(Canvas *canvas, CanvasItem *item, void *userData)
{
	register ChartItem *chartItem = (ChartItem *) item->itemData;
	register Sub *sub = (Sub *) userData;

	CallSub *callSub = dynamic_cast<CallSub *>(chartItem);
	if(callSub != NULL && callSub->GetCalledSub() == sub)
		callSub->SetCalledSub(NULL);
}

Sub::~Sub()
{
	// all CallSub-elements that reference to this object need to remove that reference
	for(size_t i = 0; i < project->GetChartsCount(); i++)
		if(project->GetChart(i) != this)
			CanvasForeachItem(project->GetChart(i)->GetCanvas(), DeleteReferences, this);

	// now delete all things from this canvas
	CanvasForeachItem(GetCanvas(), DeleteItem, NULL);
	CanvasFree(GetCanvas());
}

void Sub::IdentifyItems(Canvas *canvas, CanvasItem *item, void *userData)
{
	register int *currentId = (int *) userData;
	register ChartItem *chartItem = (ChartItem *) item->itemData;
	chartItem->SetId(++*currentId);
}

void Sub::SaveItem(Canvas *canvas, CanvasItem *item, void *userData)
{
	register XMLWriter *xml = (XMLWriter *) userData;
	register ChartItem *chartItem = (ChartItem *) item->itemData;
	
	// dependend items will be handled by the independend ones
	if(!chartItem->IsDependent())
		chartItem->WriteXML(xml);
}

void Sub::WriteXML(XMLWriter *xml)
{
	// all items need an unique, valid id for cross-references
	int firstId = 1;
	CanvasForeachItem(GetCanvas(), IdentifyItems, &firstId);
	
	xml->OpenTag("Sub");
	xml->TextTag("Name", name);

	xml->OpenTag("Blocks");
	CanvasForeachItem(GetCanvas(), SaveItem, xml);
	xml->CloseTag("Blocks");
	
	xml->CloseTag("Sub");
}

void Sub::ReadXML(XMLReader *xml, SubReferenceVector *subRefs)
{
	xml->OpenTag("Sub");
	xml->TextTag("Name", &name);
	
	xml->OpenTag("Blocks");

	IdItemMap itemIdRefs;
	while(xml->GetCurrentNode()->GetChildCount() > 0) {
		CreateFromXMLChild(xml, subRefs, &itemIdRefs);
	}

	xml->CloseTag("Blocks");

	xml->CloseTag("Sub");

	// now update refs and everything is done
	CanvasForeachItem(GetCanvas(), UpdateItemRefs, &itemIdRefs);
}

ChartItem *Sub::CreateFromXMLChild(XMLReader *xml, 
	SubReferenceVector *subRefs, IdItemMap *idRefs)
{
	ChartItem *newItem;
	string nextTag = xml->GetCurrentNode()->GetChild(0)->GetName();

	if(nextTag == "StartBlock")
		newItem = new StartBlock(this);
	else if(nextTag == "IfBlock")
		newItem = new IfBlock(this);
	else if(nextTag == "Instruction")
		newItem = new Instruction(this);
	else if(nextTag == "EndBlock")
		newItem = new EndBlock(this);
	else if(nextTag == "CallSub")
		newItem = new CallSub(this);
	else if(nextTag == "Assignment")
		newItem = new Assignment(this);
	else if(nextTag == "BlockConnector")
		newItem = new BlockConnector(this);
	else if(nextTag == "Comment")
		newItem = new Comment(this);
	else
		throw XMLReaderException("TODO: Special case");
	
	// we just have created an almost empty item... but if we call the item's 
	// ReadXML-method it will be filled with all the data of this node
	newItem->ReadXML(xml, subRefs, idRefs);

	return(newItem);
}

void Sub::UpdateItemRefs(Canvas *canvas, CanvasItem *item, void *userData)
{
	register IdItemMap *refs = (IdItemMap *) userData;
	register ChartItem *chartItem = (ChartItem *) item->itemData;
	
	// dependend items will be handled by the independend ones
	chartItem->ReadXML(refs);
}

void Sub::ClearItemIdAndCheckUnconnected(Canvas *canvas, CanvasItem *item, void *userData)
{
	register ChartItem *chartItem = (ChartItem *) item->itemData;
	chartItem->SetId(0);

	// if there is no input connector and this is not the start-block,
	// raise a warning about this unused block
	if(chartItem->GetInputConnectorCount() == 0 && 
		chartItem->HasConnectionPoint())
	{
		throw WriteBasicException(chartItem, TR_BLOCK_NOT_USED_FROM_ANYWHERE);
	}
}

void Sub::WriteBasic(BasicWriter *basic)
{

	basic->StartSub(FunctionPrefix + GetName());

	// reset the item-id for every block and check if there
	// is one that has no input-connector. If so, raise an 
	// exception (this is meant to be a warning to the user)
	CanvasForeachItem(GetCanvas(), ClearItemIdAndCheckUnconnected, NULL);
	startBlock->WriteBasic(basic);

	basic->EndSub();
}

void Sub::ClearItemLabelAndCheckUnconnected(Canvas *canvas, CanvasItem *item, void *userData)
{
	register ChartItem *chartItem = (ChartItem *) item->itemData;
	chartItem->SetLocationLabel(string());

	// if there is no input connector and this is not the start-block,
	// raise a warning about this unused block
	if(chartItem->GetInputConnectorCount() == 0 && 
		chartItem->HasConnectionPoint())
	{
		throw WriteBasicException(chartItem, TR_BLOCK_NOT_USED_FROM_ANYWHERE);
	}
}

void Sub::WriteCode(Compiler::Program &program)
{
	// reset the item-id for every block and check if there
	// is one that has no input-connector. If so, raise an 
	// exception (this is meant to be a warning to the user)
	CanvasForeachItem(GetCanvas(), ClearItemLabelAndCheckUnconnected, NULL);
	program.LabelCurrentLocation(GetName());
	startBlock->WriteCode(program);
}


EditSubDlg::EditSubDlg(NativeWindow parent, Sub *sub, ChartView *view)
{
	this->sub = sub;
	this->view = view;

	Create(parent, 330, 96, TR_EDIT_NAME);
}

void EditSubDlg::PutControls()
{
	label = new NativeLabel(this, 10, 10, 310, 21, TR_NAME_OF_THE_SUBPROGRAM);
	edit = new NativeEdit(this, 10, 35, 310, 21, sub->GetName());
	okButton = new NativeButton(this, 150, 63, 80, 24, TR_OK, 
		NativeButton::OKButton);
	cancelButton = new NativeButton(this, 240, 63, 80, 24, TR_CANCEL, 
		NativeButton::CancelButton);

	deleteButton = new NativeButton(this, 10, 63, 80, 24, TR_DELETE);
	deleteButton->onButtonClick.AddHandler(this, (NativeEventHandler) &EditSubDlg::OnDelete);
}

bool EditSubDlg::OnOK()
{
	if(!sub->GetProject()->GetMicrocontroller()->GetVariables().IsValidIdentifier(edit->GetText())) {
		NativeMessageBox(GetNativeWindow(), 
			TR_INVALID_NAME_SUPPLIED_JUST_CHARACTERS_AND_STARTING_WITH_THE_SECOND_NUMBERS_ARE_ALLOWED, 
			TR_INVALID_NAME);
		return(false);
	}

	sub->SetName(edit->GetText());

	return(true);
}

EditSubDlg::~EditSubDlg()
{
	delete label;
	delete edit;
	delete okButton;
	delete cancelButton;
	delete deleteButton;
}

void EditSubDlg::OnDelete(NativeControl *sender)
{
	Project *project = sub->GetProject();
	if(project->GetStartSub() == sub) {
		NativeMessageBox(GetNativeWindow(), TR_CAN_NOT_DELETE_START_SUB, TR_DELETE_IMPOSSIBLE);
		return;
	}

	if(!NativeYesNoMessage(GetNativeWindow(), 
		TR_DO_YOU_REALLY_WANT_TO_DELETE_THE_SUB_AND_ALL_ITS_CONTENTS, TR_DELETE))
	{
		return;
	}

	if(view->GetSub() == sub)
		view->SetSub(project->GetStartSub());
	project->DeleteChart(sub);
	FinishDialogNoOK();
}
