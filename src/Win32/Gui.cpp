#include "Gui.h"

#include <CommCtrl.h>
#include <Windowsx.h>

#include <string>
using namespace std;

bool NativeDialog::Create(NativeWindow parent, int width, int height, const std::string &title)
{
	lastDlgId = 1000;
	this->title = &title;

	backgroundBrush = CreateSolidBrush(Stock::BackgroundGrey.GetColorRef());
	captionBrush = CreateSolidBrush(Stock::Grey.GetColorRef());
	buttonBrush = CreateSolidBrush(Stock::SteelBlue.GetColorRef());
	activeButtonBrush = CreateSolidBrush(Stock::Crimson.GetColorRef());
	dialogBorder = CreatePen(PS_SOLID, FrameWidth, Stock::Grey.GetColorRef());

	LOGFONT lfont;

    memset(&lfont, 0, sizeof(lfont));
    lstrcpy(lfont.lfFaceName, "MS Shell Dlg");
    lfont.lfHeight = 16;
    lfont.lfWeight = FW_NORMAL;
    lfont.lfItalic = FALSE;
    lfont.lfCharSet = DEFAULT_CHARSET;
    lfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lfont.lfQuality = DEFAULT_QUALITY;
    lfont.lfPitchAndFamily = DEFAULT_PITCH;
	dialogFont = CreateFontIndirect(&lfont);
	
	HGLOBAL hGlobal;
	hGlobal = GlobalAlloc(GMEM_ZEROINIT, 1024);
	if(!hGlobal) 
		return(HWND_DESKTOP);

	// we must convert width and height from pixels to DialogBaseUnits...
	// to do this we use the old way: GetDialogBaseUnits, which uses the ugly Win 1 system font
	LONG baseUnitsMixed = GetDialogBaseUnits();
	width = (int) ((width + 2 * FrameWidth) * 4 / (double) LOWORD(baseUnitsMixed));
	height = (int) ((height + CaptionSize + FrameWidth) * 8 / (double) HIWORD(baseUnitsMixed));

	DLGTEMPLATE *dlgTemplate;
	dlgTemplate = (LPDLGTEMPLATE)GlobalLock(hGlobal);
	dlgTemplate->style = DS_SETFONT | DS_FIXEDSYS | WS_POPUP | WS_SYSMENU; // no frame for that window, we will do all for ourselfes
	dlgTemplate->dwExtendedStyle = 0;
	dlgTemplate->cdit = 0;
	dlgTemplate->x = 48;
	dlgTemplate->y = 48;
	dlgTemplate->cx = (short) width;
	dlgTemplate->cy = (short) height;
	INT_PTR dialog = DialogBoxIndirectParam(0, (LPDLGTEMPLATE)hGlobal, parent, (DLGPROC) NativeDialog::DlgProc, (LPARAM) this);

	GlobalFree(hGlobal);

	DeleteObject(backgroundBrush);
	DeleteObject(captionBrush);
	DeleteObject(buttonBrush);
	DeleteObject(activeButtonBrush);
	DeleteObject(dialogBorder);
	DeleteObject(dialogFont);

	return(dialog == IDOK);
}

LRESULT CALLBACK NativeDialog::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	NativeDialog *dlg = (NativeDialog *) GetWindowLongPtr(hDlg, GWL_USERDATA);
	switch (uMsg)
    {
	case WM_INITDIALOG:
		{
			// this is the only message, where dlg has an invalid value, so store the right one
			dlg = (NativeDialog *) lParam; 
			SetWindowLongPtr(hDlg, GWL_USERDATA, (LONG_PTR) lParam);
			
			SendMessage(dlg->GetNativeWindow(), WM_SETFONT, (WPARAM) dlg->dialogFont, 0);
			SetWindowText(hDlg, dlg->title->c_str());

			dlg->dialog = hDlg;
			dlg->PutControls();
		}

		return(TRUE);
		
	case WM_CTLCOLOREDIT:
		// On multiline edits we have a bug, if we set the background-mode to transparent:
		// When editing the text, the old text won't be deleted and you can't read it anymore
		// So just make it white for multiline-edits to prevent this!
		if(GetWindowStyle((HWND) lParam) & ES_MULTILINE)
			return(FALSE);
		// else fall through
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORDLG:
		SetBkMode((HDC) wParam, TRANSPARENT);
		return((LRESULT) dlg->backgroundBrush);
		
	case WM_NCHITTEST:
		{
			POINT hitPoint = { LOWORD(lParam), HIWORD(lParam) };
			ScreenToClient(hDlg, &hitPoint);
			if(hitPoint.y <= CaptionSize)
				SetWindowLong(hDlg, DWL_MSGRESULT, HTCAPTION);
			else
				SetWindowLong(hDlg, DWL_MSGRESULT, HTCLIENT);
		}
		return(TRUE);
		
	case WM_PAINT:
		{
			PAINTSTRUCT paintInfo;
			HDC hdc = BeginPaint(hDlg, &paintInfo);

			RECT rect;
			GetClientRect(hDlg, &rect);
			
			HBRUSH oldBrush = (HBRUSH) SelectObject(hdc, GetStockObject(NULL_BRUSH));
			HPEN oldPen = (HPEN) SelectObject(hdc, dlg->dialogBorder);

			Rectangle(hdc, rect.left + 2, rect.top + 2, rect.right - 1, rect.bottom - 1);

			SelectObject(hdc, oldBrush);
			SelectObject(hdc, oldPen);

			rect.bottom = CaptionSize;
			FillRect(hdc, &rect, dlg->captionBrush);

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, Stock::DarkerGrey.GetColorRef());
			char windowText[1024];
			GetWindowText(hDlg, windowText, sizeof(windowText) / sizeof(windowText[0]));

			HFONT oldFont = (HFONT) SelectObject(hdc, dlg->dialogFont);
			DrawText(hdc, windowText, strlen(windowText), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			SelectObject(hdc, oldFont);

			EndPaint(hDlg, &paintInfo);
		}
		return(TRUE);

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *drawInfo = (DRAWITEMSTRUCT *) lParam;

			HDC hdc = drawInfo->hDC;
			
			if(drawInfo->itemState & ODS_FOCUS)
				FillRect(hdc, &drawInfo->rcItem, dlg->activeButtonBrush);
			else
				FillRect(hdc, &drawInfo->rcItem, dlg->buttonBrush);
			
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, RGB(255, 255, 255));
			char windowText[1024];
			GetWindowText(drawInfo->hwndItem, windowText, sizeof(windowText) / sizeof(windowText[0]));

			DrawText(hdc, windowText, strlen(windowText), &drawInfo->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		return(TRUE);

	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK && HIWORD(wParam) == BN_CLICKED) {
			NativeDialog *dlg = (NativeDialog *) GetWindowLongPtr(hDlg, GWL_USERDATA);
			if(dlg->OnOK())
				dlg->FinishDialogWithOK();
			return(true);
		} else if(LOWORD(wParam) == IDCANCEL && HIWORD(wParam) == BN_CLICKED) {
			NativeDialog *dlg = (NativeDialog *) GetWindowLongPtr(hDlg, GWL_USERDATA);
			dlg->FinishDialogNoOK();
			return(true);
		} else {
			NativeControl *control = (NativeControl *) GetWindowLongPtr((HWND) lParam, GWL_USERDATA);
			if(control != NULL) { // MS says that it will just return NULL, if the value had been set!
				control->HandleWmCommand(HIWORD(wParam));
				return(true);
			}
		}
		break;

	case WM_TIMER:
		dlg->OnTimer();
		return(0);
	}

	return FALSE;
}

NativeControl::~NativeControl()
{
	DestroyWindow(window);
}

NativeEdit::NativeEdit(NativeDialog *dialog, int x, int y, int width, int height, const string &text, bool multiline)
{
	dialog->AdjustPos(&x, &y);
	window = CreateWindow("EDIT", text.c_str(), 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | 
			(multiline ? (ES_MULTILINE | ES_AUTOVSCROLL) : ES_AUTOHSCROLL), 
		x, y, width, height, 
		dialog->GetNativeWindow(), (HMENU) dialog->GetNextDlgId(), NULL, NULL);
	SetWindowLongPtr(window, GWL_USERDATA, (LONG_PTR) this);
	SendMessage(window, WM_SETFONT, (WPARAM) dialog->dialogFont, 0);
}

string NativeEdit::GetText()
{
	int length = GetWindowTextLength(window) + 1;
	char *buffer = (char *) malloc(length); 

	GetWindowText(window, buffer, length);
	buffer[length - 1] = 0x00;
	
	string str(buffer);
	free(buffer);

	return(str);
}

void NativeEdit::ShowBalloonTip(const string &title, const string &text, bool showErrorIcon)
{
	size_t titleLen = title.length();
	wchar_t *wTitle = (wchar_t *) malloc((titleLen + 1) * sizeof(wchar_t));
	mbstowcs(wTitle, title.c_str(), titleLen + 1);

	size_t textLen = text.length();
	wchar_t *wText = (wchar_t *) malloc((textLen + 1) * sizeof(wchar_t));
	mbstowcs(wText, text.c_str(), textLen + 1);

	EDITBALLOONTIP tipInfo;

	tipInfo.cbStruct = sizeof(EDITBALLOONTIP);
	tipInfo.pszTitle = wTitle;
	tipInfo.pszText = wText;
	tipInfo.ttiIcon = showErrorIcon ? TTI_ERROR : TTI_NONE;

	Edit_ShowBalloonTip(window, &tipInfo);

	free(wTitle);
	free(wText);
}

void NativeEdit::HandleWmCommand(WORD notificationCode)
{
	if(notificationCode == EN_CHANGE)
		onChange.CallAllHandlers(this);
}

NativeListbox::NativeListbox(NativeDialog *dialog, int x, int y, int width, int height)
{
	dialog->AdjustPos(&x, &y);
	window = CreateWindow("LISTBOX", NULL, 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | WS_BORDER,
		x, y, width, height, dialog->GetNativeWindow(), 
		(HMENU) dialog->GetNextDlgId(), NULL, NULL);
	SetWindowLongPtr(window, GWL_USERDATA, (LONG_PTR) this);
	SendMessage(window, WM_SETFONT, (WPARAM) dialog->dialogFont, 0);
}

void NativeListbox::GetText(int index, string &str)
{
	int length = SendMessage(window, LB_GETTEXTLEN, (WPARAM) index, 0);
	if(length == LB_ERR)
		return;

	char *buffer = (char *) malloc(length + 1); 

	SendMessage(window, LB_GETTEXT, (WPARAM) index, (LPARAM) buffer);
	buffer[length] = 0x00;

	str = buffer;

	free(buffer);
}

int NativeListbox::AddItem(std::string text, void *userData)
{
	int index = SendMessage(window, LB_ADDSTRING, (WPARAM) 0, (LPARAM) text.c_str());
	SendMessage(window, LB_SETITEMDATA, (WPARAM) index, (LPARAM) userData);
	return(index); 
}

void NativeListbox::SetItem(int index, std::string text, void *userData)
{ 
	SendMessage(window, WM_SETREDRAW, FALSE, 0);
	RemoveItem(index);
	SendMessage(window, LB_INSERTSTRING, index, (LPARAM) text.c_str());
	SendMessage(window, LB_SETITEMDATA, (WPARAM) index, (LPARAM) userData); 
	SendMessage(window, WM_SETREDRAW, TRUE, 0);
}

void NativeListbox::HandleWmCommand(WORD notificationCode)
{
	if(notificationCode == LBN_SELCHANGE)
		onSelectionChange.CallAllHandlers(this);
	else if(notificationCode == LBN_DBLCLK)
		onSelectionDoubleClick.CallAllHandlers(this);
}

NativeLabel::NativeLabel(NativeDialog *dialog, int x, int y, int width, int height, const string &text)
{
	dialog->AdjustPos(&x, &y);
	window = CreateWindow("STATIC", text.c_str(), WS_VISIBLE | WS_CHILD,
		x, y, width, height, dialog->GetNativeWindow(), (HMENU) 0, NULL, NULL);
	SetWindowLongPtr(window, GWL_USERDATA, (LONG_PTR) this);
	SendMessage(window, WM_SETFONT, (WPARAM) dialog->dialogFont, 0);
}

void NativeLabel::GetContent(string &str)
{
	int length = GetWindowTextLength(window) + 1;
	char *buffer = (char *) malloc(length); 

	GetWindowText(window, buffer, length);
	buffer[length - 1] = 0x00;

	str = buffer;

	free(buffer);
}

NativeButton::NativeButton(NativeDialog *dialog, int x, int y, int width, int height, 
		const string &text, SpecialButton special)
{
	dialog->AdjustPos(&x, &y);

	unsigned long id;
	if(special == CancelButton)
		id = IDCANCEL;
	else if(special == OKButton)
		id = IDOK;
	else
		id = dialog->GetNextDlgId();

	window = CreateWindow("BUTTON", text.c_str(), 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW,
		x, y, width, height, dialog->GetNativeWindow(), (HMENU) id, NULL, NULL);
	SetWindowLongPtr(window, GWL_USERDATA, (LONG_PTR) this);
	SendMessage(window, WM_SETFONT, (WPARAM) dialog->dialogFont, 0);
}

void NativeButton::HandleWmCommand(WORD notificationCode)
{
	if(notificationCode == BN_CLICKED)
		onButtonClick.CallAllHandlers(this);
}

NativeSuggest::NativeSuggest(NativeDialog *dialog, int x, int y, int width, int height, int listHeight)
{
	dialog->AdjustPos(&x, &y);
	window = CreateWindow("EDIT", "", 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_WANTRETURN,
		x, y, width, height, dialog->GetNativeWindow(), NULL, NULL, 0);
	SendMessage(window, WM_SETFONT, (WPARAM) dialog->dialogFont, 0);

	this->listHeight = listHeight;
	list = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE, "LISTBOX", "", 
		WS_POPUP | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_HASSTRINGS | WS_BORDER,
		0, 0, width, listHeight, NULL, NULL, NULL, 0);
	SetWindowLong(list, GWL_HWNDPARENT, (LONG) window);
	SendMessage(list, WM_SETFONT, (WPARAM) dialog->dialogFont, 0);

	SetWindowLongPtr(window, GWL_USERDATA, (LONG_PTR) this);
	editWndProc = (WNDPROC) GetWindowLongPtr(window, GWL_WNDPROC);
	SetWindowLongPtr(window, GWL_WNDPROC, (LONG) EditProxyStatic);

	SetWindowLongPtr(list, GWL_USERDATA, (LONG_PTR) this);
	listWndProc = (WNDPROC) GetWindowLongPtr(list, GWL_WNDPROC);
	SetWindowLongPtr(list, GWL_WNDPROC, (LONG) ListProxyStatic);
}

NativeSuggest::~NativeSuggest()
{
	DestroyWindow(window);
	DestroyWindow(list);
}

void NativeSuggest::ClearSuggestions()
{
	SendMessage(list, LB_RESETCONTENT, 0, 0);
}

void NativeSuggest::AddSuggestion(const string &text)
{
	SendMessage(list, LB_ADDSTRING, 0, (LPARAM) text.c_str());
}

void NativeSuggest::SetSuggestions(const vector<string> &suggestions)
{
	SendMessage(list, LB_RESETCONTENT, 0, 0);
	for(auto it = suggestions.begin(); it != suggestions.end(); ++it)
		SendMessage(list, LB_ADDSTRING, 0, (LPARAM) (*it).c_str());
}

string NativeSuggest::GetText() const
{
	int length = GetWindowTextLength(window) + 1;
	char *buffer = (char *) malloc(length); 

	GetWindowText(window, buffer, length);
	buffer[length - 1] = 0x00;

	string str = buffer;

	free(buffer);
	return(str);
}

LRESULT CALLBACK NativeSuggest::EditProxyStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	NativeSuggest *suggest = (NativeSuggest *) GetWindowLongPtr(hWnd, GWL_USERDATA);
	return(suggest->EditProxy(message, wParam, lParam));
}

LRESULT NativeSuggest::EditProxy(UINT message, WPARAM wParam, LPARAM lParam)
{	
	LRESULT result = 0;
	if(message == WM_GETDLGCODE) {
		// we just accept the enter key, if the list is shown... else it
		// won't be handled and could be used to activate the dialog's default
		// button
		return(IsListShown() ? DLGC_WANTALLKEYS : 0 |
			CallWindowProc(editWndProc, window, message, wParam, lParam));
	} else if(message == WM_CHAR && wParam == VK_RETURN) { // accepting current suggestion
		AcceptSuggestion();
	} else if(message == WM_CHAR && wParam == VK_ESCAPE) { // no interest in list
		HideSuggestions(false);
	} else if((message == WM_KEYDOWN || message == WM_KEYUP) && 
		(wParam == VK_UP || wParam == VK_DOWN || 
		wParam == VK_PRIOR || wParam == VK_NEXT)) 
	{
		// all keys to go up and down the list must be forwarded to the list-box
		ListProxyStatic(list, message, wParam, lParam);
	} else { // all other keys may be processed by editWndProc
		result = CallWindowProc(editWndProc, window, message, wParam, lParam);

		// if the mouse was used inside an edit or if a new char was entered show
		// suggestions. But do not show suggestions when leaving with TAB!
		if((message == WM_CHAR && wParam != VK_TAB) || message == WM_LBUTTONUP) {
			size_t textLength = GetWindowTextLength(window) + 1;
			TCHAR *text = new TCHAR[textLength];
			GetWindowText(window, text, textLength);
			
			ShowSuggestions();

			// if we find nothing this will deselect any previously selected item!
			int index = SendMessage(list, LB_FINDSTRING, (WPARAM) -1, (LPARAM) text);
			SendMessage(list, LB_SETCURSEL, index, 0);
			delete text;
		} else if(message == WM_SETFOCUS) {
			ShowSuggestions();
		} else if(message == WM_KILLFOCUS) {
			if((HANDLE) wParam != list)
				HideSuggestions(false);
		} else if(message == WM_MOUSEWHEEL) { // redirect mouse wheel to the list
			ListProxyStatic(list, message, wParam, lParam);
		}
	}

	return(result);
}
	
LRESULT CALLBACK NativeSuggest::ListProxyStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	NativeSuggest *suggest = (NativeSuggest *) GetWindowLongPtr(hWnd, GWL_USERDATA);
				
	LRESULT result = CallWindowProc(suggest->listWndProc, hWnd, message, wParam, lParam);

	// pressing return or clicking on a item accepts the selected suggestion
	if((message == WM_CHAR && wParam == VK_RETURN) ||
		(message == WM_LBUTTONUP && ListBox_GetCurSel(hWnd) != LB_ERR))
	{
		suggest->AcceptSuggestion(true);
	} else if(message == WM_CTLCOLORLISTBOX) {
		result = SendMessage(GetParent(suggest->window), message, wParam, lParam);
	}

	return(result);
}
	
void NativeSuggest::AcceptSuggestion(bool focusToEdit)
{
	int index = SendMessage(list, LB_GETCURSEL, 0, 0);
	if(index != -1) {
		int textLength = SendMessage(list, LB_GETTEXTLEN, index, 0) + 1;
		TCHAR *text = new TCHAR[textLength];
		SendMessage(list, LB_GETTEXT, index, (LPARAM) text);

		SetWindowText(window, text);
		SendMessage(window, EM_SETSEL, textLength - 1, textLength - 1);

		delete text;
	}
	HideSuggestions(focusToEdit);
}

void NativeSuggest::ShowSuggestions()
{
	// get the current position of the edit on the screen...
	RECT editRect;
	GetWindowRect(window, &editRect);
	int editHeight = editRect.bottom - editRect.top;
	int editWidth = editRect.right - editRect.left;
				
	POINT point = { 0 , 0 };
	ClientToScreen(window, &point);
	suggestRect.left = point.x;
	suggestRect.top = point.y;
	suggestRect.right = point.x + editWidth;
	suggestRect.bottom = point.y + editHeight + listHeight - 1;

	// and move it accordingly
	SetWindowPos(list, NULL, point.x, point.y + editHeight - 1, 
		suggestRect.right - suggestRect.left, listHeight, SWP_NOACTIVATE);
	ShowWindow(list, SW_SHOWNOACTIVATE);

	// update all list of suggestions
	onShowSuggestions.CallAllHandlers(this);
	
	// if no mouse-hook is running, start it
	if(mouseHook == NULL) {
		currentHookNativeSuggest = this;
		mouseHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC) MouseHookProc, GetModuleHandle(NULL), 0);
	}
}

void NativeSuggest::HideSuggestions(bool focusToEdit)
{
	// deactivate the mouse-hook, if it exists
	if(mouseHook) {
		UnhookWindowsHookEx(mouseHook);
		currentHookNativeSuggest = NULL;
		mouseHook = NULL;
	}

	// if needed, focus the edit instead of the list (or a random other control)
	if(focusToEdit)
		SetFocus(window);

	ShowWindow(list, SW_HIDE);
}

// We use this hook to get even those clicks outside the Edit or the ListBox
// If a click was made outside, hide the suggestion-window and disable this
// hook
LRESULT CALLBACK NativeSuggest::MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode >= 0 && (wParam == WM_RBUTTONDOWN || wParam == WM_LBUTTONDOWN) && 
		currentHookNativeSuggest != NULL) 
	{
		POINT p;
		GetCursorPos(&p);

		if(!PtInRect(&suggestRect, p))
			currentHookNativeSuggest->HideSuggestions();
	}
		
	return(CallNextHookEx(mouseHook, nCode, wParam, lParam)); 
}

// although all instances share these variables it is no problem, as there
// is only one suggestion-edit expanded at once!
NativeSuggest *NativeSuggest::currentHookNativeSuggest = NULL;
HHOOK NativeSuggest::mouseHook = NULL;
RECT NativeSuggest::suggestRect = { 0, 0, 0, 0 };
