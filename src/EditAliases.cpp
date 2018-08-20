#include "EditAliases.h"

#include <string>
#include <vector>
using namespace std;

#include "Project.h"
#include "Translate.h"

class AliasDialog : public NativeDialog
{
public:
	AliasDialog(NativeWindow parent, const string &oldName, Variables *variables);
	~AliasDialog();

	virtual void PutControls();
	virtual bool OnOK();

	inline const string &GetNewName() const
		{ return(newName); };

	void OnSuggest(NativeControl *sender);

private:
	NativeEdit *nameEdit;
	NativeSuggest *targetSuggest;
	NativeButton *okButton;
	NativeButton *cancelButton;

	Variables *variables;
	string oldName;
	string newName;
};

AliasDialog::AliasDialog(NativeWindow parent, const string &oldName, Variables *variables)
	: variables(variables), oldName(oldName)
{
	Create(parent, 340, 220, TR_EDIT_ALIAS);
}

AliasDialog::~AliasDialog()
{
	delete nameEdit;
	delete targetSuggest;
	delete okButton;
	delete cancelButton;
}

void AliasDialog::PutControls()
{
	nameEdit = new NativeEdit(this, 20, 20, 310, 21, oldName);
	targetSuggest = new NativeSuggest(this, 20, 50, 310, 21);
	targetSuggest->SetText(variables->GetAliasDestination(oldName));
	targetSuggest->onShowSuggestions.AddHandler(this, (NativeEventHandler) &AliasDialog::OnSuggest);
	okButton = new NativeButton(this, 110, 180, 100, 24, TR_OK, NativeButton::OKButton);
	cancelButton = new NativeButton(this, 230, 180, 100, 24, TR_CANCEL, NativeButton::CancelButton);
}

void AliasDialog::OnSuggest(NativeControl *sender)
{
	targetSuggest->SetSuggestions(variables->SuggestAliasTargets(targetSuggest->GetText(), oldName));
}

bool AliasDialog::OnOK()
{
	newName = nameEdit->GetText();
	if(oldName.empty())
		return(variables->AddAlias(newName, targetSuggest->GetText()));
	else
		return(variables->UpdateAlias(oldName, newName, targetSuggest->GetText()));
}

EditAliasesDialog::EditAliasesDialog(NativeWindow parent, Variables *variables)
	: variables(variables)
{
	Create(parent, 285, 485, TR_EDIT_ALIASES);
}

EditAliasesDialog::~EditAliasesDialog(void)
{
	delete explanation;
	for(size_t i = 0; i < (size_t) aliasList->GetItemCount(); i++)
		delete (string *) aliasList->GetUserData(i);
	delete aliasList;
	delete okButton;
	delete newButton;
}

void EditAliasesDialog::PutControls()
{
	explanation = new NativeLabel(this, 20, 15, 250, 100, TR_EDIT_ALIASES_EXPLANATION);
	aliasList = new NativeListbox(this, 20, 130, 250, 300);
	aliasList->onSelectionChange.AddHandler(this, 
		(NativeEventHandler) &EditAliasesDialog::OnChangeCurrent);
	
	auto allAliases = variables->GetAllAliases();
	for(auto it = allAliases.begin(); it != allAliases.end(); ++it)
		aliasList->AddItem(*it + " > " + variables->GetAliasDestination(*it), (void *) new string(*it));
	
	newButton = new NativeButton(this, 20, 445, 100, 24, TR_NEW);
	newButton->onButtonClick.AddHandler(this, (NativeEventHandler) &EditAliasesDialog::OnNew);
	okButton = new NativeButton(this, 170, 445, 100, 24, TR_OK, NativeButton::OKButton);
}

bool EditAliasesDialog::OnOK()
{
	return(true);
}

void EditAliasesDialog::OnChangeCurrent(NativeControl *sender)
{
	int index = aliasList->GetSelectedIndex();
	if(index == -1)
		return;

	string *aliasName = (string *) aliasList->GetUserData(aliasList->GetSelectedIndex());
	AliasDialog aliasDlg(GetNativeWindow(), *aliasName, variables);
	if(aliasDlg.WasOK()) {
		aliasList->SetItem(index, 
			aliasDlg.GetNewName() + " > " + variables->GetAliasDestination(aliasDlg.GetNewName()), 
			(void *) new string(aliasDlg.GetNewName()));
		delete aliasName;
	}
}

void EditAliasesDialog::OnNew(NativeControl *sender)
{
	AliasDialog aliasDlg(GetNativeWindow(), "", variables);
	if(aliasDlg.WasOK()) {
		string newName = aliasDlg.GetNewName();
		aliasList->AddItem(newName + " > " + variables->GetAliasDestination(newName), (void *) new string(newName));
	}
}
