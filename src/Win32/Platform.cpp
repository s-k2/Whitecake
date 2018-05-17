#include "Platform.h"

#include <CommCtrl.h>
#include <ShlObj.h>

#include <string>
using std::string;
#include <vector>
using std::vector;

// Use the safe-functions whenever possible!
#if !defined(_MSC_VER) || _MSC_VER < 1400
#define strcpy_s(a, b, c) strcpy((a), (c))
#define sprintf_s snprintf // TODO: Make this work without any issue
#endif /* _MSC_VER < 1400 */

// but disable those warnings

class CursorInit
{
public:
	HCURSOR arrowCursor;
	HCURSOR crossCursor;
	CursorInit()
	{
		crossCursor = LoadCursor(NULL, IDC_CROSS);
		arrowCursor = LoadCursor(NULL, IDC_ARROW);
	}
};
CursorInit cursorInit;

void NativeSetPointerCursor(NativeWindow window)
{
	SetCursor(cursorInit.arrowCursor);
	SetClassLong(window, GCL_HCURSOR, (DWORD) cursorInit.arrowCursor);
}

void NativeSetCrossCursor(NativeWindow window)
{
	SetCursor(cursorInit.crossCursor);
	SetClassLong(window, GCL_HCURSOR, (DWORD) cursorInit.crossCursor);
}

bool NativeIsWindowMaximized(NativeWindow window)
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(window, &wp);

	return(wp.showCmd == SW_MAXIMIZE);
}

bool NativeFileDialog(NativeWindow parent, const string &extension, 
	const string &extDescription, string &path, bool open)
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof (ofn);
	ofn.hwndOwner = parent;

	TCHAR szPathBuffer[MAX_PATH];
	ofn.lpstrFile = szPathBuffer;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szPathBuffer);

	TCHAR filterBuffer[256];
	if(extDescription.length() + extension.length() > 
		sizeof(filterBuffer) - sizeof("\0*.\0\0"))
	{
		return(false);
	}
	if(!open)
		ofn.lpstrDefExt = extension.c_str();

	// it's horrible to create that filter with nul-values as seperators of the
	// different parts... but to statisfy microsft's we do it...
	strcpy_s(filterBuffer, 256, extDescription.c_str());
	int curPos = extDescription.size();
	curPos++;
	strcpy_s(filterBuffer + curPos, 256 - curPos, extension.c_str());
	curPos += extension.size();
	filterBuffer[++curPos] = 0x00; // double zero
	ofn.lpstrFilter = filterBuffer;

	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir= NULL;
	ofn.Flags = open ? OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST : OFN_OVERWRITEPROMPT;

	if((open && GetOpenFileName(&ofn) != 0) ||
		(!open && GetSaveFileName(&ofn) != 0)) {
		path = ofn.lpstrFile;
		return(true);
	}
	return(false);
}

NativeProcess::NativeProcess(const string &path, vector<string> args)
{
	size_t argsStrLength = path.size() + 1;
	for(vector<string>::const_iterator it = args.begin(); it != args.end(); it++)
		argsStrLength += (*it).size() + 1;

	char *argsStr = (char *) malloc(argsStrLength);
	strcpy(argsStr, path.c_str());
	for(vector<string>::const_iterator it = args.begin(); it != args.end(); it++) {
		strcat(argsStr, " ");
		strcat(argsStr, (*it).c_str());
	}

	STARTUPINFO startupInfo;
	memset(&startupInfo, 0, sizeof(startupInfo));
	memset(&process, 0, sizeof(process));

	CreateProcess(path.c_str(), argsStr, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &process);

	free(argsStr);
}

NativeProcess::NativeProcess(const string &path)
{
	STARTUPINFO startupInfo;
	memset(&startupInfo, 0, sizeof(startupInfo));
	memset(&process, 0, sizeof(process));

	CreateProcess(path.c_str(), NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &process);

}

NativeProcess::~NativeProcess()
{
	CloseHandle(process.hProcess);
	CloseHandle(process.hThread);
}

int NativeProcess::GetExitCode()
{
	DWORD exitCode;
	if(GetExitCodeProcess(process.hProcess, &exitCode) == false)
		return(-1);

	if(exitCode == STILL_ACTIVE)
		return(-1);

	return(exitCode);
}

NativeThreadStarter::NativeThreadStarter(ThreadFunction function, void *userData)
{
	userMain = function;
	this->userData = userData;
	thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadMain, this, 0, NULL);
}

void NativeThreadStarter::Join()
{
	WaitForSingleObject(thread, INFINITE);
}

DWORD NativeThreadStarter::ThreadMain(LPVOID threadStarter)
{
	NativeThreadStarter *starter = (NativeThreadStarter *) threadStarter;

	starter->userMain(starter->userData);

	return(0);
}

bool NativeDeleteFullDirectory(string path)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;

	if(path.back() != '\\')
		path.push_back('\\');

	string pattern = path + "*";
	hFind = FindFirstFile(pattern.c_str(), &findData);

	if(hFind == INVALID_HANDLE_VALUE)
		return(false);

	string subPath; // put here to avoid unnecessary buffer realloations...
	do {
		subPath = path + findData.cFileName;

		if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if(findData.cFileName != string(".") && findData.cFileName != string("..")) {
				// delete subdirectories using recursion
				if(!NativeDeleteFullDirectory(subPath.c_str()))
					return(false);
			}
		} else {
			if(!DeleteFile(subPath.c_str()))
				return(false);
		}
	} while(FindNextFile(hFind, &findData) != false);

	FindClose(hFind);

	return(RemoveDirectory(path.c_str()) == TRUE);
}

bool NativeGetAppDataDir(string &path)
{
	TCHAR pathBuffer[MAX_PATH + 1];

	if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, pathBuffer))) {
		path = pathBuffer;

		if(path.back() == '\\')
			path.pop_back();

		return(true);
	}

	return(false);
}

void NativeCreateTempDirPath(string &path)
{
	TCHAR tmpPathBuffer[MAX_PATH + 1];

	int ret = GetTempPath(MAX_PATH, tmpPathBuffer);
	if(ret == 0 || ret == sizeof(tmpPathBuffer) || !NativeFileExists((path = tmpPathBuffer))) {
		// fake a temporary path in the application directory
		NativeGetExecutablePath(path);
		path.erase(path.find_last_of('\\'));
		path.append("\\tmp");
	}
	if(path.back() != '\\')
		path.push_back('\\');

	string executableName;
	NativeGetExecutablePath(executableName);
	executableName.erase(0, executableName.find_last_of("\\"));
	if(executableName.front() == '\\')
		executableName.erase(0, 1);

	do {
		// it is no problem if srand has never been called before... we just 
		// want to have any number and try as long as the name doesn't exist
		sprintf_s(tmpPathBuffer, sizeof(tmpPathBuffer), "-%.8x", rand() % INT_MAX);
	} while(NativeFileExists(path + executableName + tmpPathBuffer));

	path = path + executableName + tmpPathBuffer;
}

void NativeGetExecutablePath(string &path)
{
	TCHAR pathBuffer[MAX_PATH + 1];
	if(GetModuleFileName(NULL, pathBuffer, sizeof(pathBuffer)) == sizeof(pathBuffer)) {
		// we have a problem here... but at least make the string valid
		pathBuffer[MAX_PATH] = 0x00;
	}
	path = pathBuffer;
	
	if(path.substr(0, 4) == "\\\\?\\")
		path.erase(0, 4);
}

size_t NativeShowMenu(NativeWindow toplevel, int x, int y, const vector<string> &entries)
{
	// display a menu created using CreateMenu()
	HMENU hMenu = ::CreatePopupMenu();
	if (NULL != hMenu)
	{
		RECT clientRect;
		GetClientRect(toplevel, &clientRect);

		POINT point;
		point.x = x;
		point.y = y;
		ClientToScreen(toplevel, &point);

		// special-case for maximized window... add 8 pixels in each direction
		if(NativeIsWindowMaximized(toplevel)) {
			point.x += 4;
			point.y += 4;
		}

		int FirstMenuId = 15000;
		for(size_t i = 0; i < entries.size(); i++) {
			if(entries[i] == "-")
				AppendMenu(hMenu, MF_SEPARATOR, (size_t) -1, NULL);
			else
				AppendMenu(hMenu, MF_STRING, FirstMenuId + i, entries[i].c_str());
		}

		int sel = TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, point.x, point.y, toplevel, NULL);
		DestroyMenu(hMenu);

		if(sel == 0)
			sel = -1;
		else
			sel -= FirstMenuId;
		
		return(sel);
	}

	return((size_t) -1);
}

const TCHAR NativePathSeparator = '\\';