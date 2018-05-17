
#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#ifndef WIN32
#error "This file cannot be included on a platform other than Win32"
#endif /* WIN32 */

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /* _MSC_VER */

#include <windows.h>
#include <string>
#include <vector>
#include <utility>

#ifdef _MSC_VER
#pragma warning(pop)
#endif /* _MSC_VER */

#include "Drawing.h"
#include "Serial.h"

#define MODIFIER_SHIFT MK_SHIFT
#define MODIFIER_CTRL MK_CONTROL
#define MODIFIER_LEFT_MOUSE_BUTTON MK_LBUTTON
#define MODIFIER_RIGHT_MOUSE_BUTTON MK_RBUTTON

#define KEY_SHIFT VK_SHIFT
#define KEY_CTRL VK_CONTROL
#define KEY_LEFT_MOUSE_BUTTON VK_LBUTTON
#define KEY_RIGHT_MOUSE_BUTTON VK_RBUTTON
#define KEY_ALT VK_MENU
#define KEY_DEL VK_DELETE
#define IsKeyPressed(key) (GetKeyState(key) & 0x80)

typedef HWND NativeWindow;
inline void NativeWindowForceRedraw(NativeWindow window)
	{ InvalidateRect(window, NULL, false); };
inline void NativeGrabMouse(NativeWindow window)
	{ SetCapture(window); };
inline void NativeUngrabMouse(NativeWindow)
	{ ReleaseCapture(); };

extern void NativeSetPointerCursor(NativeWindow window);
extern void NativeSetCrossCursor(NativeWindow window);

inline void NativeCloseWindow(NativeWindow window)
	{ SendMessage(window, WM_CLOSE, 0, 0); };
inline void NativeMaximizeWindow(NativeWindow window)
	{ ShowWindow(window, SW_MAXIMIZE); };
inline void NativeRestoreWindow(NativeWindow window)
	{ ShowWindow(window, SW_RESTORE); };
inline void NativeMinimizeWindow(NativeWindow window)
	{ ShowWindow(window, SW_MINIMIZE); };
bool NativeIsWindowMaximized(NativeWindow window);

extern bool NativeFileDialog(NativeWindow parent, const std::string &extension, 
	const std::string &extDescription, std::string &path, bool open);

inline bool NativeFileOpenDialog(NativeWindow parent, const std::string &extension, 
	const std::string &extDescription, std::string &path)
	{ return(NativeFileDialog(parent, extension, extDescription, path, true)); };

inline bool NativeFileSaveDialog(NativeWindow parent, const std::string &extension, 
	const std::string &extDescription, std::string &path)
	{ return(NativeFileDialog(parent, extension, extDescription, path, false)); };

inline void NativeMessageBox(NativeWindow parent, const std::string &text, const std::string &title)
	{ MessageBox(parent, text.c_str(), title.c_str(), MB_ICONINFORMATION | MB_OK); };
inline bool NativeYesNoMessage(NativeWindow parent, const std::string &text, const std::string &title)
	{ return(MessageBox(parent, text.c_str(), title.c_str(), MB_ICONQUESTION | MB_YESNO) == IDYES); };
enum NativeMessageAnswer { NativeYes, NativeNo, NativeCancel };
inline NativeMessageAnswer NativeYesNoCancelMessage(NativeWindow parent, const std::string &text, const std::string &title)
{ 
	int res = MessageBox(parent, text.c_str(), title.c_str(), MB_ICONQUESTION | MB_YESNOCANCEL);
	switch(res) {
	case IDYES:
		return(NativeYes);
	case IDNO:
		return(NativeNo);
	default:
		return(NativeCancel);
	}
}


#include "Gui.h"

class NativeProcess
{
public:
	NativeProcess(const std::string &path, std::vector<std::string> args);
	explicit NativeProcess(const std::string &path);
	~NativeProcess();

	inline void WaitForTermination()
		{ WaitForSingleObject(process.hProcess, INFINITE); };

	int GetExitCode();

private:
	PROCESS_INFORMATION process;
};

// if this object is destroyed, the thread will survive
class NativeThreadStarter
{
	typedef void (*ThreadFunction)(void *userData);
public:
	NativeThreadStarter(ThreadFunction function, void *userData);

	void Join();

private:
	static DWORD ThreadMain(LPVOID threadStarter);

	HANDLE thread;
	ThreadFunction userMain;
	void *userData;
};

extern bool NativeDeleteFullDirectory(std::string path);

inline bool NativeFileExists(const std::string &path)
{
	DWORD attributes = GetFileAttributes(path.c_str());
	
	return(attributes != INVALID_FILE_ATTRIBUTES);
}

inline bool NativeCreateDirectory(const std::string &path)
	{ return(CreateDirectory(path.c_str(), NULL) == TRUE); };

// path will never end with an '\\'
void NativeCreateTempDirPath(std::string &path);

inline void NativeSleep(int milliseconds)
	{ Sleep(milliseconds); };

// returns the path where we may create folders for app-data (in windows the AppData-directory)
extern bool NativeGetAppDataDir(std::string &path);

// returns the path of the executable of the running process
extern void NativeGetExecutablePath(std::string &path);

// returns index of selected entry or -1 if no menu-entry was clicked
extern size_t NativeShowMenu(NativeWindow toplevel, int x, int y, const std::vector<std::string> &entries);

extern const TCHAR NativePathSeparator;

inline time_t NativeGetLinktime()
{
	IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER *) GetModuleHandle(NULL);
	IMAGE_NT_HEADERS *ntHeader = (IMAGE_NT_HEADERS *) ((char *) dosHeader + dosHeader->e_lfanew);
	return(ntHeader->FileHeader.TimeDateStamp);
}


#endif /* WIN32_PLATFORM_H */