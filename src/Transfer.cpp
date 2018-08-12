#include "Transfer.h"

#include <stdlib.h>
#include <string.h>
#include <string>
using std::string;
#include <vector>
using std::vector;

#include "ChartItem.h"
#include "CodeWriteException.h"
#include "Platform.h"
#include "Project.h"
#include "Settings.h"
#include "Translate.h"
#include "Uploader.h"

#include "BascomUploader.h"
#include "ArduinoUploader.h"

#include "Compiler.h"

class CompilerException
{
public:
	explicit CompilerException(const std::string &message)
		{ this->message = message; };
	explicit CompilerException(const char *message)
		{ this->message = message; };

	inline const std::string &GetMessage()
		{ return(message); };

private:
	std::string message;
};

class TransferException
{
public:
	explicit TransferException(const std::string &message)
		{ this->message = message; };
	explicit TransferException(const char *message)
		{ this->message = message; };

	inline const std::string &GetMessage()
		{ return(message); };

private:
	std::string message;
};

class TransferDialog : public NativeDialog
{
public:
	explicit TransferDialog(NativeWindow parent, Transfer *transfer);
	~TransferDialog();

	virtual void PutControls();
	virtual bool OnOK();
	virtual void OnTimer();

private:
	NativeButton *button;
	NativeLabel *resetLabel;
	Transfer *transfer;
};

TransferDialog::TransferDialog(NativeWindow parent, Transfer *transfer)
{
	this->transfer = transfer;
	Create(parent, 480, 150, TR_TRANSFER);
}

void TransferDialog::PutControls()
{
	button = new NativeButton(this, 190, 110, 100, 24, TR_CANCEL, NativeButton::OKButton);
	resetLabel = new NativeLabel(this, 10, 10, 400, 20, TR_GENERATING_CODE);

	SetTimer(100);
}

TransferDialog::~TransferDialog()
{
	delete button;
	delete resetLabel;
}

bool TransferDialog::OnOK()
{
	transfer->threadStop = 1;
	return(true);
}

void TransferDialog::OnTimer()
{
	if(transfer->threadState == 0)
		FinishDialogWithOK();
	else if(transfer->threadState == Transfer::CodeGen)
		resetLabel->SetContent(TR_GENERATING_CODE);
	else if(transfer->threadState == Transfer::Compilation)
		resetLabel->SetContent(TR_COMPILING_CODE);
	else if(transfer->threadState == Transfer::WaitForReset)
		resetLabel->SetContent(TR_PLEASE_DO_RESET);
	else if(transfer->threadState == Transfer::Sending)
		resetLabel->SetContent(TR_SENDING_PROGRAM);
}

Transfer::Transfer(Project *project, NativeWindow parent)
	: offendingItem(NULL)
{
	this->project = project;

	threadState = CodeGen;
	threadStop = 0;

	try {
		programToSend.reset(new Compiler::Program());
		project->WriteCode(*programToSend);
		programToSend->Finish();
		programToSend->Dump();
	} catch(CodeWriteException e) {
		NativeMessageBox(parent, e.GetText(), TR_TRANSFER_ERROR);
		offendingItem = e.GetItem();
		return;
	} catch(std::runtime_error e) {
		NativeMessageBox(parent, string("Internal compiler error:\n") + e.what(), TR_COMPILER_ERROR);
		return;
	}

	// now create the worker thread that handles the communication with the board
	NativeThreadStarter thread(Transfer::SendThread, this);

	TransferDialog dialog(parent, this);
	thread.Join();

	if(!compileErrorMessage.empty()) // show error messages only if not aborted
		NativeMessageBox(parent, compileErrorMessage, TR_TRANSFER_ERROR);
	else if(!sendErrorMessage.empty())
		NativeMessageBox(parent, sendErrorMessage, TR_TRANSFER_ERROR);
}

void Transfer::SendThread(void *transferVoid)
{
	Transfer *transfer = (Transfer *) transferVoid;

	transfer->threadState = Compilation;

	try {
		if(theSettings.GetProgrammer() == "TinyBas")
			BascomUploader uploader(theSettings.GetSerialPort(), theSettings.GetSerialBaud(), 
				transfer->programToSend->GetBinary(), &transfer->threadStop, &transfer->threadState);
		else
			ArduinoUploader uploader(theSettings.GetSerialPort(), theSettings.GetSerialBaud(), 
				transfer->programToSend->GetBinary(), &transfer->threadStop, &transfer->threadState);
	} catch(UploaderException e) {
		transfer->sendErrorMessage = e.GetMessage();
	}

	transfer->threadState = Done;
}

