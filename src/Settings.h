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
	inline int GetSerialBaud() const
		{ return(serialBaud); };
	inline void SetSerialBaud(int value)
		{ serialBaud = value; changed = true; };

	inline const std::string &GetCompilerPath() const
		{ return(compilerPath); };
	inline void SetCompilerPath(const std::string &value)
		{ compilerPath = value; changed = true; };

	inline const std::string &GetRegFileName() const
		{ return(regFileName); };
	inline int GetCrystalFreqency() const
		{ return(crystalFrequency); };
	inline int GetHardwareStack() const
		{ return(hardwareStack); };
	inline int GetSoftwareStack() const
		{ return(softwareStack); };
	inline int GetFrameSize() const
		{ return(frameSize); };
	inline bool GetUsingHardwareUART() const
		{ return(usingHardwareUART); };
	inline const std::string &GetSoftUARTIn()
		{ return(softUARTIn); };
	inline const std::string &GetSoftUARTOut()
		{ return(softUARTOut); };

	inline const std::vector<std::string> &GetIntegerVariables() const
		{ return(integerVariables); };
	const std::vector<std::string> &GetByteVariables() const
		{ return(byteVariables); };;
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
	int serialBaud;

	std::string compilerPath;

	std::string regFileName;
	int crystalFrequency;
	int hardwareStack;
	int softwareStack;
	int frameSize;
	bool usingHardwareUART;
	std::string softUARTIn;
	std::string softUARTOut;
	
	std::vector<std::string> integerVariables;
	std::vector<std::string> byteVariables;
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
	NativeLabel *serialBaudLabel;
	NativeLabel *compilerPathLabel;
	NativeEdit *serialPortEdit;
	NativeEdit *serialBaudEdit;
	NativeEdit *compilerPathEdit;
	NativeButton *okButton;
	NativeButton *cancelButton;
};


#endif /* SETTINGS_H */