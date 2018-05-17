#include "Platform.h"

class Project;
class Variables;

class EditAliasesDialog : public NativeDialog
{
public:
	EditAliasesDialog(NativeWindow parentm, Project *project);
	~EditAliasesDialog();

	virtual void PutControls();
	virtual bool OnOK();
	
	void OnChangeCurrent(NativeControl *sender);
	void OnNew(NativeControl *sender);

private:
	NativeLabel *explanation;
	NativeListbox *aliasList;
	NativeButton *newButton;
	NativeButton *okButton;

	Variables &variables;
};