#ifndef SETTINGS_H
#define SETTINGS_H

#include "Platform.h"

#include <string>
#include <vector>
#include <utility>

class Settings
{
public:
	Settings();
	~Settings();

	inline const std::string &GetProgrammer() const
		{ return(programmer); };
	inline const std::string &GetSerialPort() const
		{ return(serialPort); };
	inline void SetSerialPort(const std::string &value)
		{ serialPort = value; changed = true; };

	inline const std::vector<std::string> &GetIntegerVariables() const
		{ return(integerVariables); };
	const std::vector<std::string> &GetIOPorts() const
		{ return(ioPorts); };;
	const std::vector<std::string> &GetIOPins() const
		{ return(ioPins); };;
	const std::vector<std::string> &GetADCChannels() const
		{ return(adcChannels); };
	const std::vector<std::pair<std::string, std::string>> &GetAliases() const
		{ return(aliases); };


private:
	std::string settingsPath;
	bool changed;
	
	std::string programmer;
	std::string serialPort;

	std::vector<std::string> integerVariables;
	std::vector<std::string> ioPorts;
	std::vector<std::string> ioPins;
	std::vector<std::string> adcChannels;
	std::vector<std::pair<std::string, std::string>> aliases;

	void SetDefaults();
	void ReadConfigIfExists();
	void OnKey(std::string key, std::string value);
	void WriteConfig();
};

extern Settings theSettings;

class SettingsDialog : public NativeDialog
{
public:
	explicit SettingsDialog(NativeWindow parent);
	~SettingsDialog();

	virtual void PutControls();
	virtual bool OnOK();

private:
	NativeLabel *serialPortLabel;
	NativeEdit *serialPortEdit;
	NativeButton *okButton;
	NativeButton *cancelButton;
};

#endif /* SETTINGS_H */
