#include "Settings.h"

#include <fstream>
#include <iostream>
using std::ifstream;
#include <string>
using std::string;
#include <utility>
using std::pair;
#include <vector>
using std::vector;

#include "Config.h"
#include "Helper/String.h"
#include "Platform.h"
#include "Translate.h"

Settings theSettings; // = new Settings();

Settings::Settings()
{
	// at first we set the default values (even if they are not set in the ini)
	SetDefaults();

	// overwrite those defaults if some options could be read from a file
	ReadConfigIfExists();
	changed = false;	
}

void Settings::SetDefaults()
{

#ifdef WHITECAKE_FOR_ARDUINO
	programmer = "Arduino";
	serialPort = "COMx";
	serialBaud = 115200;


#endif /* WHITECAKE_FOR_ARDUINO */

#if defined (WHITECAKE_FOR_TINYBAS) || defined (WHITECAKE_FOR_TINYTICK)
	programmer = "TinyBas";
	serialPort = "HID";
	serialBaud = 2400;
#endif /* defined (WHITECAKE_FOR_TINYBAS) || defined (WHITECAKE_FOR_TINYTICK) */

	for(size_t i = 0; i < 32; i++)
		integerVariables.push_back("var" + String::IntToStr(i));
	for(size_t i = 0; i < 8; i++)
		ioPorts.push_back("PORTB." + String::IntToStr(i));
	for(size_t i = 6; i < 7; i++)
		ioPorts.push_back("PORTC." + String::IntToStr(i));
	for(size_t i = 2; i < 8; i++)
		ioPins.push_back("PIND." + String::IntToStr(i));
	for(size_t i = 0; i < 6; i++)
		adcChannels.push_back("ADC" + String::IntToStr(i));

	NativeGetExecutablePath(compilerPath);
	compilerPath.erase(compilerPath.find_last_of(NativePathSeparator));
	compilerPath = compilerPath + NativePathSeparator + "bascomp.exe";
}

void Settings::ReadConfigIfExists()
{
	// first check if there is a ini file in the same path as the executable
	NativeGetExecutablePath(settingsPath);
	settingsPath += ".ini";

	// if not use a user-secific file
	if(!NativeFileExists(settingsPath)) {
		NativeGetAppDataDir(settingsPath);
		settingsPath = settingsPath + NativePathSeparator + "Whitecake" + NativePathSeparator + "Whitecake.ini";

		// if this file does not exist, just use defaults
		if(!NativeFileExists(settingsPath)) {
			settingsPath = "";
			return;
		}
	}

	// if we found an existing settings file, we read from it
	ifstream in(settingsPath.c_str());
	string currentLine;

	while(getline(in, currentLine)) {
		string key, value;

		size_t dividerPos = currentLine.find('=');

		if(dividerPos != string::npos) { // TODO: Put this in extra function
			key = currentLine.substr(0, dividerPos);
			key.erase(0, key.find_first_not_of(" \t"));
			key.erase(key.find_last_not_of(" \t") + 1);

			value = currentLine.substr(dividerPos + 1);
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);

			OnKey(key, value);
		} else {
			NativeMessageBox(NULL, TR_INVALID_LINE_IN_CONFIGURATION_FILE + currentLine, TR_ERROR_WITH_CONFIGURATION);
		}
	}
}

void Settings::OnKey(string key, string value)
{
	if(key == "programmer") {
		programmer = value;
	} else if(key == "serialPort") {
		serialPort = value;
	} else if(key == "serialBaud") {
		if(!String::ToInt(value, 10, &serialBaud)) {
			NativeMessageBox(NULL, TR_FOUND_INVALID_KEY_ + key + TR__WITH_VALUE_ + value + 
				TR__IN_CONFIGURATION_FILEI_WILL_IGNORE_IT, TR_CONFIGURATION_ERROR);
		}
	} else if(key == "compilerPath") {
		compilerPath = value;
	} else if(key == "integerVariables") {
		integerVariables.clear();
		String::Tokenize(value, integerVariables, ' ');
	} else if(key == "ioPorts") {
		ioPorts.clear();
		String::Tokenize(value, ioPorts, ' ');
	} else if(key == "ioPins") {
		ioPins.clear();
		String::Tokenize(value, ioPins, ' ');
	} else if(key == "adcChannels") {
		adcChannels.clear();
		String::Tokenize(value, adcChannels, ' ');
	} else if(key == "aliases") {
		vector<string> allAliases; // a list of all aliases, but each item needs to be split again
		String::Tokenize(value, allAliases, ',');
		for(size_t i = 0; i < allAliases.size(); i++) {
			vector<string> currentAlias; // first item should be alias, second original name
			
			String::Tokenize(allAliases[i], currentAlias, ':');
			if(currentAlias.size() < 2) {
				NativeMessageBox(NULL, TR_FOUND_INVALID_KEY_ + key + TR__WITH_VALUE_ + value + 
					TR__IN_CONFIGURATION_FILEI_WILL_IGNORE_IT, TR_CONFIGURATION_ERROR);
				aliases.clear(); // if one alias contains an error, ignore all
				return;
			}
			
			String::Trim(currentAlias[0]);
			String::Trim(currentAlias[1]);
			if(currentAlias[0].empty() || currentAlias[1].empty()) {
				NativeMessageBox(NULL, TR_FOUND_INVALID_KEY_ + key + TR__WITH_VALUE_ + value + 
					TR__IN_CONFIGURATION_FILEI_WILL_IGNORE_IT, TR_CONFIGURATION_ERROR);
				aliases.clear();
				return;
			}

			aliases.push_back(pair<string, string>(currentAlias[0], currentAlias[1]));
		}
	} else {
		NativeMessageBox(NULL, TR_FOUND_INVALID_KEY_ + key + TR__WITH_VALUE_ + value + 
			TR__IN_CONFIGURATION_FILEI_WILL_IGNORE_IT, TR_CONFIGURATION_ERROR);
	}
}

Settings::~Settings(void)
{
	if(changed)
		WriteConfig();
}

void Settings::WriteConfig()
{
	FILE *out = NULL;

	// if we opened a file try to open it for writing
	if(!settingsPath.empty()) {
		if((out = fopen(settingsPath.c_str(), "w")) == NULL) {
			NativeMessageBox(NULL, TR_COULD_NOT_SAVE_CHANGED_PROGRAM_SETTINGS_INTO_ + settingsPath, TR_ERROR_WITH_SETTINGS);
			return;
		}
	} else {
		// first try program path
		NativeGetExecutablePath(settingsPath);
		settingsPath += ".ini";

		if((out = fopen(settingsPath.c_str(), "w")) == NULL) {
			// then user-specific
			NativeGetAppDataDir(settingsPath);
			settingsPath = settingsPath + NativePathSeparator + "Whitecake";
			
			if(!NativeFileExists(settingsPath))
				// we ignore an error here, because we get it if we try to open a file
				// in an directory that has not been created
				NativeCreateDirectory(settingsPath);
			
			settingsPath = settingsPath + NativePathSeparator + "Whitecake.ini";
			if((out = fopen(settingsPath.c_str(), "w")) == NULL) {
				NativeMessageBox(NULL, TR_COULD_NOT_SAVE_CHANGED_PROGRAM_SETTINGS_INTO_ + settingsPath, TR_ERROR_WITH_SETTINGS);
				return;
			} 
		}
	}
	
	fprintf(out, "programmer = %s\n", programmer.c_str());
	fprintf(out, "serialPort = %s\n", serialPort.c_str());
	fprintf(out, "serialBaud = %d\n", serialBaud);
	// write the compiler path only, if it is not in the current directory (to be an portable-app)
	string compilerInProgramDir;
	NativeGetExecutablePath(compilerInProgramDir);
	compilerInProgramDir.erase(compilerInProgramDir.find_last_of(NativePathSeparator));
	compilerInProgramDir = compilerInProgramDir + NativePathSeparator + "bascomp.exe";
	if(compilerPath != compilerInProgramDir)
		fprintf(out, "compilerPath = %s\n", compilerPath.c_str());
	fprintf(out, "integerVariables = %s\n", String::Join(integerVariables, " ").c_str());
	fprintf(out, "ioPorts = %s\n", String::Join(ioPorts, " ").c_str());
	fprintf(out, "ioPins = %s\n", String::Join(ioPins, " ").c_str());
	fprintf(out, "adcChannels = %s\n", String::Join(adcChannels, " ").c_str());
	string aliasesStr;
	for(size_t i = 0; i < aliases.size(); i++) {
		if(i > 0)
			aliasesStr.append(", ");
		aliasesStr.append(aliases[i].first);
		aliasesStr.append(":");
		aliasesStr.append(aliases[i].second);
	}
	fprintf(out, "aliases = %s\n", aliasesStr.c_str());
	fclose(out);
}

SettingsDialog::SettingsDialog(NativeWindow parent)
{
	Create(parent, 480, 150, TR_PROGRAM_SETTINGS);
}

SettingsDialog::~SettingsDialog(void)
{
	delete serialPortLabel;
/*	delete serialBaudLabel;
	delete compilerPathLabel; */
	delete serialPortEdit;
/*	delete serialBaudEdit;
	delete compilerPathEdit;*/
	delete okButton;
	delete cancelButton;
}

void SettingsDialog::PutControls()
{
	serialPortLabel = new NativeLabel(this, 20, 10, 95, 21, TR_SERIAL_PORT);
	serialPortEdit = new NativeEdit(this, 120, 10, 350, 21, theSettings.GetSerialPort());
/*	serialBaudLabel = new NativeLabel(this, 20, 40, 95, 21, TR_BAUDRATE);
	serialBaudEdit = new NativeEdit(this, 120, 40, 350, 21, String::IntToStr(theSettings.GetSerialBaud()));
	compilerPathLabel = new NativeLabel(this, 20, 70, 95, 21, TR_COMPILER_PATH);
	compilerPathEdit = new NativeEdit(this, 120, 70, 350, 21, theSettings.GetCompilerPath());*/

	okButton = new NativeButton(this, 260, 110, 100, 24, TR_OK, NativeButton::OKButton);
	cancelButton = new NativeButton(this, 370, 110, 100, 24, TR_CANCEL, NativeButton::CancelButton);
}

bool SettingsDialog::OnOK()
{
	// we start with baud in case there is an error neither serialPort nor compilerPath would be changed
/*	int serialBaud;
	if(!String::ToInt(serialBaudEdit->GetText(), 10, &serialBaud)) {
		serialBaudEdit->ShowBalloonTip(TR_INVALID_NUMBER, TR_YOU_HAVE_ENTERED_AN_INVALID_NUMBER, true);
		return(false);
	}*/

	theSettings.SetSerialPort(serialPortEdit->GetText());
//	theSettings.SetCompilerPath(compilerPathEdit->GetText());

	return(true);
}
