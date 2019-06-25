#include "Instruction.h"

#include <string>
using std::string;

#include "BlockConnector.h"
#include "Expressions.h"
#include "CodeWriteException.h"
#include "Helper/Geometry.h"
#include "Helper/String.h"
#include "Helper/XMLReader.h"
#include "Helper/XMLWriter.h"
#include "Project.h"
#include "Sub.h"
#include "Translate.h"
#include "Config.h"

class EditInstructionDlg : public NativeDialog
{
public:
	EditInstructionDlg(NativeWindow parent, Instruction *instruction);
	~EditInstructionDlg();

	virtual void PutControls();
	virtual bool OnOK();

	void OnListboxDblClick(NativeControl *sender);

private:
	Instruction *instruction;
	struct InstructionInfo *info;

	NativeLabel *labels[InstructionMaxArgs + 1]; // +1 due to the return-field
	NativeEdit *editFields[InstructionMaxArgs + 1]; // +1 due to the return-field
	NativeListbox *listboxes[InstructionMaxArgs + 1]; // +1 due to the return-field
	NativeButton *okButton;

};

InstructionInfo Instruction::InstructionInfos[] = 
{
	{ 
		"print_str", "Zeichenkette ausgeben", "", 1,
		{
			{ FixedString, "Zeichenkette" },
			{ NoArg, NULL }, 
			{ NoArg, NULL } 
		},
		NoRet 
	},
	{ 
		"print_number", "Zahl ausgeben", "", 1,
		{
			{ FixedInteger | IntegerVariable, TR_NUMBER },
			{ NoArg, NULL }, 
			{ NoArg, NULL } 
		},
		NoRet 
	},
	{ 
		"print_newline", "Neue-Zeile ausgeben", "", 0,
		{
			{ NoArg, NULL }, 
			{ NoArg, NULL }, 
			{ NoArg, NULL } 
		},
		NoRet 
	},

	{ 
		"adc_read", TR_GET_ADC, "", 1,
		{
			{ ADCChannel, TR_CHANNEL },
			{ NoArg, NULL }, 
			{ NoArg, NULL } 
		},
		IntegerVariable 
	},
	{ 
		"waitchar", TR_WAIT_FOR_KEY, "", 0,
		{
			{ NoArg, NULL }, 
			{ NoArg, NULL }, 
			{ NoArg, NULL } 
		}, 
		IntegerVariable,
	},
	{ 
		"recvchar", TR_READ_KEY, "", 0,
		{
			{ NoArg, NULL }, 
			{ NoArg, NULL }, 
			{ NoArg, NULL } 
		}, 
		IntegerVariable,
	},
	{ 
		"print_char", "Sende Zeichen", "", 1,
		{
			{ FixedInteger | IntegerVariable, "Zeichen" }, 
			{ NoArg, NULL }, 
			{ NoArg, NULL } 
		}, 
		NoRet,
	},
#ifdef WHITECAKE_FOR_ARDUINO
	{ 
		"servo_output", "Servo-Wert ausgeben", "", 2,
		{
			{ FixedInteger, "Servo-Kanal (0 oder 1)" }, 
			{ FixedInteger | IntegerVariable, "Wert" }, 
			{ NoArg, NULL } 
		}, 
		NoRet,
	},
	{ 
		"pwm_output", "PWM-Wert ausgeben", "", 2,
		{
			{ FixedInteger, "PWM-Kanal (0 oder 1)" }, 
			{ FixedInteger | IntegerVariable, "Wert" }, 
			{ NoArg, NULL } 
		}, 
		NoRet,
	},
#endif /* WHITECAKE_FOR_ARDUINO */
#ifdef WHITECAKE_FOR_TINYTICK
	{ 
		"pwm_init", "PWM aktivieren", "", 0,
		{
			{ NoArg, NULL }, 
			{ NoArg, NULL }, 
			{ NoArg, NULL } 
		}, 
		NoRet,
	},
	{ 
		"pwm_set", "PWM-Wert ausgeben", "", 2,
		{
			{ FixedInteger, "PWM-Kanal (0, 1 oder 2)" }, 
			{ FixedInteger | IntegerVariable, "Wert" }, 
			{ NoArg, NULL } 
		}, 
		NoRet,
	},
	{ 
		"servo_init", "Servo aktivieren", "", 0,
		{
			{ NoArg, NULL }, 
			{ NoArg, NULL }, 
			{ NoArg, NULL } 
		}, 
		NoRet,
	},
	{ 
		"servo_set", "Servo-Wert ausgeben", "", 2,
		{
			{ FixedInteger, "Servo-Kanal (0, 1 oder 2)" }, 
			{ FixedInteger | IntegerVariable, "Wert" }, 
			{ NoArg, NULL } 
		}, 
		NoRet,
	},
#endif /* WHITECAKE_FOR_TINYTICK */
	{ 
		"waitms", TR_WAIT_MILLISECONDS, "", 1,
		{
			{ FixedInteger | IntegerVariable, TR_WAIT_TIME }, 
			{ NoArg, NULL }, 
			{ NoArg, NULL } 
		}, 
		NoRet,
	},
	{ 
		"waitus", TR_WAIT_MICROSECONDS, "", 1,
		{
			{ FixedInteger | IntegerVariable, TR_WAIT_TIME }, 
			{ NoArg, NULL }, 
			{ NoArg, NULL } 
		}, 
		NoRet,
	}
};

const size_t Instruction::InstructionInfosCount = 
	sizeof(Instruction::InstructionInfos) / sizeof(InstructionInfo);

const int Instruction::Width = 160;
const int Instruction::Height = 72;
const double Instruction::Shift = 0.15;
const int Instruction::ShiftValue = (int) (Instruction::Height * 0.15);

Instruction::Instruction(Sub *sub, int x, int y, InstructionInfo *info)
	: ProgramBlock(sub, x, y, Width, Height)
{
	dependentItems[0] = new BlockConnector(sub, x + Width / 2, y + Height);
	dependentItems[1] = NULL;
	this->info = info;
}

Instruction::Instruction(Sub *sub)
	: ProgramBlock(sub, Width, Height)
{
	dependentItems[0] = dependentItems[1] = NULL;
	this->info = NULL;
}

Instruction::~Instruction(void)
{
}

void Instruction::OnPaint(Drawing *drawing, int flags)
{
	drawing->FillParallelogram(0, 0, Width, Height, Shift,
		flags & PaintNotSelected ? Stock::LighterGrey : Stock::CornflowerBlue);

	string infoStr = info->text;
	infoStr += "\n";
	for(size_t i = 0; i < GetInstructionInfo()->argsCount; i++)
		infoStr += (i > 0 ? string(", ") : string(" (")) + (GetArg(i).empty() ? string("?") : GetArg(i));
	if(GetInstructionInfo()->argsCount)
		infoStr += ")";
	if(GetInstructionInfo()->ret != NoRet)
		infoStr += " -> " + (GetResult().empty() ? string("?") : GetResult());

	drawing->PrintText(Width * 0.05, 0, Width * 0.85, Height, infoStr, Stock::GuiFont,  
		flags & PaintNotSelected ? Stock::DarkGrey : Stock::White);

	if(flags & PaintErrorMark)
		drawing->DrawEllipse(-20, -20, Width + 40, Height + 40, Stock::ThickerCrimsonPen);
}

bool Instruction::IsInItem(int x, int y)
{
	return(Parallelogram::IntersectsWithPoint<Width, Height, ShiftValue>
		(x - GetX(), y - GetY()));
}

bool Instruction::IsInItem(int x, int y, int width, int height)
{
	return(Parallelogram::IntersectWithRectanglePart<Width, Height, ShiftValue>
		(x, y, width, height, CanvasItemGetX(&item), CanvasItemGetY(&item)));
}

bool Instruction::OnEdit(NativeWindow parent)
{
	EditInstructionDlg dlg(parent, this);
	return(dlg.WasOK());
}

void Instruction::WriteXML(XMLWriter *xml)
{
	xml->OpenTag("Instruction");
	ChartItem::WriteXML(xml);
	
	xml->TextTag("InstructionId", info->idString);
	for(size_t i = 0; i < info->argsCount; i++) {
		string tagName = "Argument" + String::IntToStr(i);
		xml->TextTag(tagName.c_str(), args[i]);
	}
	xml->TextTag("Result", result);
	xml->CloseTag("Instruction");
}

void Instruction::ReadXML(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *idRefs)
{
	xml->OpenTag("Instruction");
	ChartItem::ReadXML(xml, subRefs, idRefs);
	
	string instructionId;
	xml->TextTag("InstructionId", &instructionId);
	
	size_t i;
	for(i = 0; i < InstructionInfosCount; i++) {
		if(InstructionInfos[i].idString == instructionId)
			break;
	}
	if(i == InstructionInfosCount)
		return; // TODO: throw some exception

	info = &InstructionInfos[i];
	for(i = 0; i < info->argsCount; i++) {
		string tagName = "Argument" + String::IntToStr(i);
		xml->TextTag(tagName, &args[i]);
	}
	xml->TextTag("Result", &result);

	xml->CloseTag("Instruction");
}

void Instruction::WriteCode(Compiler::Program &program)
{
	std::vector<Compiler::ValueExpression> args;
	for(size_t i = 0; i < info->argsCount; i++) {
		ParsedParameter arg(GetProject()->GetVariables(), this->args[i]);
		if(!arg.IsValid(info->args[i].type))
			throw CodeWriteException(this, TR_PARAMTER_MISSING_IN_INSTRUCTION);
		args.emplace_back(arg.ToValueExpression());
	}

	if(info->ret == NoRet)
		program.Call(Compiler::UnknownAddress(info->idString), args);
	else {
		if(GetResult().empty())
			throw CodeWriteException(this, TR_PARAMTER_MISSING_IN_INSTRUCTION);
		string resultVariableNoAlias = GetProject()->GetVariables().GetAliasDestination(GetResult());
		if(resultVariableNoAlias.empty())
			resultVariableNoAlias = GetResult();
		program.Call(Compiler::UnknownAddress(info->idString), args, Compiler::UnknownAddress(resultVariableNoAlias));
	}

	ChartItem::WriteCode(program);
}

EditInstructionDlg::EditInstructionDlg(NativeWindow parent, Instruction *instruction)
	: instruction(instruction), info(instruction->GetInstructionInfo())
{
	int rowCount = info->argsCount + (info->ret != NoRet ? 1 : 0);
	if(rowCount > 0)
		Create(parent, rowCount * 200, 330, TR_INSTRUCTION);
}

void EditInstructionDlg::PutControls()
{
	size_t i;
	for(i = 0; i < info->argsCount; i++) {
		labels[i] = new NativeLabel(this, 10 + i * 200, 10, 180, 21, info->args[i].text);
		editFields[i] = new NativeEdit(this, 10 + i * 200, 32, 180, 18, instruction->GetArg(i));
		listboxes[i] = new NativeListbox(this, 10 + i * 200, 60, 180, 220);
		listboxes[i]->onSelectionDoubleClick.AddHandler(this, (NativeEventHandler) &EditInstructionDlg::OnListboxDblClick);

		std::vector<std::string> possibleOperands = instruction->GetProject()->GetVariables().FormatOperands(info->args[i].type);
		for(auto it = possibleOperands.begin(); it != possibleOperands.end(); ++it)
			listboxes[i]->AddItem(*it);
	}

	if(info->ret != NoRet) {
		labels[i] = new NativeLabel(this, 10 + i * 200, 10, 180, 21, TR_RESULT);
		editFields[i] = new NativeEdit(this, 10 + i * 200, 32, 180, 18, instruction->GetResult());
		listboxes[i] = new NativeListbox(this, 10 + i * 200, 60, 180, 220);
		listboxes[i]->onSelectionDoubleClick.AddHandler(this, (NativeEventHandler) &EditInstructionDlg::OnListboxDblClick);

		std::vector<std::string> possibleOperands = instruction->GetProject()->GetVariables().FormatOperands(info->ret);
		for(auto it = possibleOperands.begin(); it != possibleOperands.end(); ++it)
			listboxes[i]->AddItem(*it);
	}

	okButton = new NativeButton(this, (info->argsCount + (info->ret != NoRet ? 1 : 0)) * 200 - 110, 
		290, 100, 24, TR_OK, NativeButton::OKButton);
}

bool EditInstructionDlg::OnOK()
{
	size_t i;
	for(i = 0; i < info->argsCount; i++) {
		ParsedParameter parameter(instruction->GetProject()->GetVariables(), editFields[i]->GetText());
		if(!parameter.IsValid(info->args[i].type)) {
			//editFields[i]->ShowBalloonTip(TR_WRONG_DATA, errorMessage, true);
			return(false);
		}

		instruction->SetArg(i, parameter.GetText());
	}
	if(info->ret != NoRet) {
		ParsedParameter parameter(instruction->GetProject()->GetVariables(), editFields[i]->GetText());
		if(!parameter.IsValid(info->ret)) {
			//editFields[i]->ShowBalloonTip(TR_WRONG_DATA, errorMessage, true);
			return(false);
		}
		instruction->SetResult(parameter.GetText());
	}

	return(true);
}

void EditInstructionDlg::OnListboxDblClick(NativeControl *sender)
{
	NativeListbox *listbox = (NativeListbox *) sender;
	NativeEdit *edit = NULL;

	size_t i;
	for(i = 0; i < info->argsCount; i++) {
		if(listboxes[i] == listbox)
			edit = editFields[i];
	}
	if(info->ret != NoRet && listboxes[i] == listbox) {
		edit = editFields[i];
	}

	string text;
	listbox->GetText(listbox->GetSelectedIndex(), text);
	edit->SetText(text);
}

EditInstructionDlg::~EditInstructionDlg()
{
	size_t rowCount = info->argsCount + (info->ret != NoRet ? 1 : 0);

	// if nothing has been showed we do not need to do anything
	if(rowCount < 1)
		return;

	for(size_t i = 0; i < rowCount; i++) {
		delete labels[i];
		delete editFields[i];
		delete listboxes[i];
	}
	delete okButton;
}

SelectInstructionDlg::SelectInstructionDlg(NativeWindow parent)
{
	Create(parent, 215, 562, TR_SELECT_INSTRUCTION);
}

SelectInstructionDlg::~SelectInstructionDlg()
{
	delete listbox;
}

void SelectInstructionDlg::PutControls()
{
	listbox = new NativeListbox(this, 10, 10, 200, 542);
	// TODO: FIXME
	listbox->SetItemHeight(30);
	
	for(size_t i = 0; i < Instruction::InstructionInfosCount; i++) {
		listbox->AddItem(Instruction::InstructionInfos[i].text, (void *) &(Instruction::InstructionInfos[i]));
	}

	listbox->onSelectionChange.AddHandler(this, (NativeEventHandler) &SelectInstructionDlg::OnSelectionChange);
}

bool SelectInstructionDlg::OnOK()
{
	int selIndex = listbox->GetSelectedIndex();
	if(selIndex >= 0 && (size_t) selIndex < Instruction::InstructionInfosCount) {
		selectedInstruction = (InstructionInfo *) listbox->GetUserData(selIndex);
		return(true);
	}

	return(false);
}

void SelectInstructionDlg::OnSelectionChange(NativeControl *sender)
{
	if(OnOK())
		FinishDialogWithOK();
}
