#ifndef TRANSFER_H
#define TRANSFER_H

#include <string>
#include <vector>
#include <memory>

#include "Platform.h"
#include "Compiler.h"

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
	std::unique_ptr<Compiler::Program> programToSend;
	

	int threadStop; // 0 if thread may run, if GUI wants stop 1
	int threadState; // > 0 if it's still running! 0 means done
	enum State { Done, CodeGen, Compilation, WaitForReset, Sending };

	std::string compileErrorMessage;
	std::string sendErrorMessage;

	static void SendThread(void *transferVoid);
	void Compile();

	friend class TransferDialog;
};

#endif /* TRANSFER_H */
