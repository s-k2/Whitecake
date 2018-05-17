#include "Transfer.h"

#include <stdlib.h>
#include <string.h>
#include <string>
using std::string;
#include <vector>
using std::vector;

#include "Platform.h"
#include "Project.h"
#include "Settings.h"
#include "Translate.h"
#include "Uploader.h"
#include "Helper/BasicWriter.h"

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



// we put creation/deletion of the temporary directory in an extra class, 
// else we would have no possibility to delete it in case of an exception. 
// Though all exceptions will be thrown within the constructor Transfer() but 
// only fully constructed objects would be destroyed again, there is no way 
// to delete the directory in ~Transfer() so we do it here, because this 
// object will be created completly and thus be destroyed in case of an exception!
struct TmpDirCreator
{
	std::string *path;
	TmpDirCreator(std::string &path)
	{
		if(!NativeCreateDirectory(path))
			throw CompilerException(TR_COULD_NOT_CREATE_DIRECTORY);
		this->path = &path;
	}

	~TmpDirCreator()
	{
		// Just for debugging.... NativeMessageBox(NULL, TR_FINISHED_EXPORT_PRESS_OK_TO_DELETE_ALL_TEMPORARY_FILES, TR_CONTINUE);
		NativeDeleteFullDirectory(*path);		
	}
};

vector<unsigned char> tmpBinary;

Transfer::Transfer(Project *project, NativeWindow parent)
	: offendingItem(NULL)
{
	this->project = project;

	NativeCreateTempDirPath(tmpDir);
	codePath = tmpDir + NativePathSeparator + "code.bas";
	binFile = tmpDir + NativePathSeparator + "code.bin";
	errorFile = tmpDir + NativePathSeparator + "code.err";

	threadState = CodeGen;
	threadStop = 0;

	TmpDirCreator tmpDirCreator(tmpDir);
	try {
		Compiler::Program program;
		project->WriteCode(program);
		program.Finish();
		program.Dump();
		tmpBinary = program.GetBinary();
	} catch(WriteBasicException e) {
		NativeMessageBox(parent, e.GetText(), TR_TRANSFER_ERROR);
		offendingItem = e.GetItem();
		return;
	} catch(std::runtime_error e) {
		NativeMessageBox(parent, string("Internal compiler error:\n") + e.what(), TR_COMPILER_ERROR);
		return;
	}

	// now create the worker thread that constantly receives and sends data
	NativeThreadStarter thread(Transfer::CompileAndSendThread, this);

	TransferDialog dialog(parent, this);

	thread.Join();

	if(!compileErrorMessage.empty()) // show error messages only if not aborted
		NativeMessageBox(parent, compileErrorMessage, TR_TRANSFER_ERROR);
	else if(!sendErrorMessage.empty())
		NativeMessageBox(parent, sendErrorMessage, TR_TRANSFER_ERROR);
}

void Transfer::CompileAndSendThread(void *transferVoid)
{
	Transfer *transfer = (Transfer *) transferVoid;

	transfer->threadState = Compilation;
	/*try {
		transfer->Compile();
	} catch(CompilerException e) {
		transfer->compileErrorMessage = e.GetMessage();
		transfer->threadState = 0;
		return;
	}*/

	try {
		vector<unsigned char> program;
//		if(!transfer->ReadBinary(program))
//			throw UploaderException(TR_COULD_NOT_READ_PROGRAM_FILE);
		program = tmpBinary;

		if(theSettings.GetProgrammer() == "TinyBas")
			BascomUploader uploader(theSettings.GetSerialPort(), theSettings.GetSerialBaud(), 
				program, &transfer->threadStop, &transfer->threadState);
		else
			ArduinoUploader uploader(theSettings.GetSerialPort(), theSettings.GetSerialBaud(), 
				program, &transfer->threadStop, &transfer->threadState);
	} catch(UploaderException e) {
		transfer->sendErrorMessage = e.GetMessage();
	}

	transfer->threadState = Done;
}

void Transfer::Compile()
{
	vector<string> args;
	args.push_back(codePath);
	args.push_back("auto");
	NativeProcess process(theSettings.GetCompilerPath(), args);
	process.WaitForTermination();

	if(!NativeFileExists(binFile)) {
		FILE *messageFile;

		if(NativeFileExists(errorFile) && (messageFile = fopen(errorFile.c_str(), "r")) != NULL) {
			fseek(messageFile, 0, SEEK_END);
			string errorMessage;
			errorMessage.resize(ftell(messageFile));
			rewind(messageFile);
			fread(&errorMessage[0], 1, errorMessage.size(), messageFile);
			fclose(messageFile);

			NativeMessageBox(NULL, errorMessage, TR_COMPILER_ERROR);

			throw CompilerException(TR_COMPILER_ERROR + errorMessage);
		} else {
			throw CompilerException(TR_COULD_NOT_COMPILE_CODE_FOR_UNKNOWN_REASON);
		}
	}
}

bool Transfer::ReadBinary(vector<unsigned char> &out)
{
	FILE *f = fopen(binFile.c_str(), "rb");
	if(f == NULL)
		return(false);

	if(fseek(f, 0, SEEK_END) != 0)
		return(false);

	size_t length = ftell(f);
	out.resize(length);
	
	rewind(f);

	bool ok = fread(&out[0], length, 1, f) == 1;
	
	fclose(f);
	return(ok);
}
