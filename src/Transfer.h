#ifndef TRANSFER_H
#define TRANSFER_H

#include <string>
#include <vector>

#include "Platform.h"

class Project;
class ChartItem;

class Transfer
{
public:
	explicit Transfer(Project *project, NativeWindow parent);

	inline ChartItem *GetOffendingItem()
		{ return(offendingItem); };
	
private:
	Project *project;
	ChartItem *offendingItem;

	std::string tmpDir;
	std::string codePath;
	std::string binFile;
	std::string errorFile;

	int threadStop; // 0 if thread may run, if GUI wants stop 1
	int threadState; // > 0 if it's still running! 0 means done
	enum State { Done, CodeGen, Compilation, WaitForReset, Sending };

	std::string compileErrorMessage;
	std::string sendErrorMessage;

	static void CompileAndSendThread(void *transferVoid);
	void Compile();
	bool ReadBinary(std::vector<unsigned char> &out);

	friend class TransferDialog;
};

#endif /* TRANSFER_H */