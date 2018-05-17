#ifndef GUI_H
#define GUI_H

#include "Platform.h"

//extern void *CreateNativeSuggest(HWND parent, int x, int y, int width, int height, int listHeight);

class NativeDialog
{
public:
	inline bool WasOK()
		{ return(isOK); };

protected:
	bool Create(NativeWindow parent, int width, int height, const std::string &title = std::string(""));

	virtual void PutControls() = 0;
	virtual bool OnOK() 
		{ return(true); };

	inline void FinishDialogWithOK()
		{ isOK = true; EndDialog(dialog, 0); };
	inline void FinishDialogNoOK()
		{ isOK = false; EndDialog(dialog, 0); };

	inline void SetTimer(int milliseconds)
	{ ::SetTimer(dialog, 1, milliseconds, NULL); };
	virtual void OnTimer()
		{ };
	
	inline NativeWindow GetNativeWindow()
		{ return(dialog); };
private:
	NativeWindow dialog;

	static LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LONG lastDlgId;
	inline LONG GetNextDlgId()
		{ return(++lastDlgId); };

	HFONT dialogFont;

	bool isOK;

	const std::string *title;

	HBRUSH backgroundBrush, captionBrush, buttonBrush, activeButtonBrush;
	HPEN dialogBorder;

	static const int FrameWidth = 4;
	static const int CaptionSize = 24;
	inline void AdjustPos(int *x, int *y)
		{ x += FrameWidth; *y += CaptionSize; };
	
	friend class NativeButton;
	friend class NativeEdit;
	friend class NativeListbox;
	friend class NativeLabel;
	friend class NativeSuggest;
};

class NativeControl;
typedef void (NativeDialog::*NativeEventHandler)(NativeControl *);
class NativeEventDispatcher
{
private:
	typedef std::vector<std::pair<NativeDialog *, NativeEventHandler> > HandlersList;

	HandlersList handlers;
public:
	inline void AddHandler(NativeDialog *object, NativeEventHandler handler)
		{ handlers.push_back(std::make_pair(object, handler)); };

	inline void CallAllHandlers(NativeControl *sender)
	{ 
		for(HandlersList::iterator it = handlers.begin(); it != handlers.end(); it++)
			(((*it).first)->*((*it).second))(sender);
	};

};

class NativeControl
{
public:
	virtual ~NativeControl();

protected:
	NativeControl()
	{
	}

	NativeWindow window;
	virtual void HandleWmCommand(WORD notificationCode) 
		{ 
		};

	friend class NativeDialog;
};

class NativeEdit : public NativeControl
{
public:
	NativeEdit(NativeDialog *dialog, int x, int y, int width, int height, const std::string &text, bool multiline = false);

	std::string GetText();
	inline void SetText(const std::string &str)
		{ SetWindowText(window, str.c_str()); };

	inline void PutAtSelection(const std::string &str)
		{ SendMessage(window, EM_REPLACESEL, TRUE, (LPARAM) str.c_str()); };

	void ShowBalloonTip(const std::string &title, const std::string &text, bool showErrorIcon = false);

	// All events - we support ;-)
	NativeEventDispatcher onChange;

protected:
	virtual void HandleWmCommand(WORD notificationCode);
};

class NativeListbox : public NativeControl
{
public:
	NativeListbox(NativeDialog *dialog, int x, int y, int width, int height);

	int AddItem(std::string text, void *userData = 0);
	void SetItem(int index, std::string text, void *userData = 0);
    inline void RemoveItem(int index)
		{ SendMessage(window, LB_DELETESTRING, index, 0); };
	inline void *GetUserData(int index)
		{ return((void *) SendMessage(window, LB_GETITEMDATA, index, 0)); };
	void GetText(int index, std::string &str);

	inline int GetSelectedIndex()
		{ return(SendMessage(window, LB_GETCURSEL, 0, 0)); };
	inline void SetSelectedIndex(int index)
		{ SendMessage(window, LB_SETCURSEL, (WPARAM) index, 0); };
	inline int GetItemCount()
		{ return(SendMessage(window, LB_GETCOUNT, 0, 0)); };

	// this is just used by one dialog... do we need it?
	inline void SetItemHeight(int height)
		{ SendMessage(window, LB_SETITEMHEIGHT, 0, height); };

	// All events - we support ;-)
	NativeEventDispatcher onSelectionChange;
	NativeEventDispatcher onSelectionDoubleClick;

protected:
	virtual void HandleWmCommand(WORD notificationCode);
};

class NativeLabel : public NativeControl
{
public:
	NativeLabel(NativeDialog *dialog, int x, int y, int width, int height, const std::string &text);

	void GetContent(std::string &str);
	inline void SetContent(std::string str)
		{ SetWindowText(window, str.c_str()); };
};

class NativeButton : public NativeControl
{
public:
	enum SpecialButton { NotSpecial, OKButton, CancelButton };
	NativeButton(NativeDialog *dialog, int x, int y, int width, int height, 
		const std::string &text, SpecialButton special = NotSpecial);

	inline void SetLabel(std::string str)
		{ SetWindowText(window, str.c_str()); };

	// All events - we support ;-)
	NativeEventDispatcher onButtonClick;

protected:
	virtual void HandleWmCommand(WORD notificationCode);
};

class NativeSuggest : public NativeControl
{
public:
	NativeSuggest(NativeDialog *dialog, int x, int y, int width, int height, int listHeight = 100);
	~NativeSuggest();

	void ClearSuggestions();
	void AddSuggestion(const std::string &str);

	void SetSuggestions(const std::vector<std::string> &suggestions);

	std::string GetText() const;
	inline void SetText(const std::string &str)
		{ SetWindowText(window, str.c_str()); };
	
	NativeEventDispatcher onShowSuggestions;
private:
	// window is the handle to the edit-control
	HWND list;
	WNDPROC editWndProc;
	WNDPROC listWndProc;
	int listHeight;
	
	// although this data is static, there is no problem! Only one window can have 
	// the focus and thus only one control has a running hook!
	static HHOOK mouseHook;
	static NativeSuggest *currentHookNativeSuggest;
	static RECT suggestRect;

	static LRESULT CALLBACK EditProxyStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT EditProxy(UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK ListProxyStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void AcceptSuggestion(bool focusToEdit = false);
	void ShowSuggestions();
	void HideSuggestions(bool focusToEdit = true);
	static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);

	inline bool IsListShown() const
		{ return(mouseHook != NULL); };
};

#endif /* GUI_H */