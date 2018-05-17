#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /* _MSC_VER */

#include <windows.h>
#include <windowsx.h>
#include <Commctrl.h>
#include "Resource.h"

#include <DbgHelp.h>

#include "../MainWindow.h"
#include "Drawing.h"

#include <dwmapi.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif /* _MSC_VER */

HINSTANCE hInst;
const TCHAR *szTitle = "Whitecake v0.9";// (Beta) - [" __DATE__ "]";
const TCHAR *szWindowClass = "MainWindow";

MainWindow *mainWindow; // TODO: Bad way, find a better solution for it
int mainWindowButtonState = 0;
HDC hMemoryDC = NULL; // to improve drawing operations
HBITMAP hBitmap = NULL;
bool recreateMemoryDC = true; // to create the DC on the first OnPaint-call

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void OnPaint(HWND hWnd);

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{ 
    BITMAP bmp; 
    PBITMAPINFO pbmi; 
    WORD    cClrBits; 

    // Retrieve the bitmap color format, width, and height.  
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
        return(NULL);//errhandler("GetObject", hwnd); 

    // Convert the color format to a count of bits.  
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 

    // Allocate memory for the BITMAPINFO structure. (This structure  
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
    // data structures.)  

    if (cClrBits < 24) 
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
        sizeof(BITMAPINFOHEADER) + 
        sizeof(RGBQUAD) * (1<< cClrBits)); 

    // There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 

    else 
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, 
        sizeof(BITMAPINFOHEADER)); 

    // Initialize the fields in the BITMAPINFO structure.  

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    pbmi->bmiHeader.biWidth = bmp.bmWidth; 
    pbmi->bmiHeader.biHeight = bmp.bmHeight; 
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes; 
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel; 
    if (cClrBits < 24) 
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 

    // If the bitmap is not compressed, set the BI_RGB flag.  
    pbmi->bmiHeader.biCompression = BI_RGB; 

    // Compute the number of bytes in the array of color  
    // indices and store the result in biSizeImage.  
    // The width must be DWORD aligned unless the bitmap is RLE 
    // compressed. 
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8
        * pbmi->bmiHeader.biHeight; 
    // Set biClrImportant to 0, indicating that all of the  
    // device colors are important.  
    pbmi->bmiHeader.biClrImportant = 0; 
    return pbmi; 
} 

void CreateBMPFile(const char *pszFile, PBITMAPINFO pbi, 
                   HBITMAP hBMP, HDC hDC) 
{ 
    HANDLE hf;                 // file handle  
    BITMAPFILEHEADER hdr;       // bitmap file-header  
    PBITMAPINFOHEADER pbih;     // bitmap info-header  
    LPBYTE lpBits;              // memory pointer
    DWORD cb;                   // incremental count of bytes  
    BYTE *hp;                   // byte pointer  
    DWORD dwTmp; 

    pbih = (PBITMAPINFOHEADER) pbi; 
    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    if (!lpBits) 
        return;//errhandler("GlobalAlloc", hwnd); 

    // Retrieve the color table (RGBQUAD array) and the bits  
    // (array of palette indices) from the DIB.  
    if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, 
        DIB_RGB_COLORS)) 
    {
        return;//errhandler("GetDIBits", hwnd); 
    }

    // Create the .BMP file.  
    hf = CreateFile(pszFile, 
        GENERIC_READ | GENERIC_WRITE, 
        (DWORD) 0, 
        NULL, 
        CREATE_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, 
        (HANDLE) NULL); 
    if (hf == INVALID_HANDLE_VALUE) 
        return;//errhandler("CreateFile", hwnd); 
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
    // Compute the size of the entire file.  
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
        pbih->biSize + pbih->biClrUsed 
        * sizeof(RGBQUAD) + pbih->biSizeImage); 
    hdr.bfReserved1 = 0; 
    hdr.bfReserved2 = 0; 

    // Compute the offset to the array of color indices.  
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
        pbih->biSize + pbih->biClrUsed 
        * sizeof (RGBQUAD); 

    // Copy the BITMAPFILEHEADER into the .BMP file.  
    if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), 
        (LPDWORD) &dwTmp,  NULL)) 
    {
        return;//errhandler("WriteFile", hwnd); 
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
    if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) 
        + pbih->biClrUsed * sizeof (RGBQUAD), 
        (LPDWORD) &dwTmp, ( NULL)))
        return;//errhandler("WriteFile", hwnd); 

    // Copy the array of color indices into the .BMP file.  
    cb = pbih->biSizeImage; 
    hp = lpBits; 
    if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL)) 
        return;//errhandler("WriteFile", hwnd); 

    // Close the .BMP file.  
    if (!CloseHandle(hf)) 
        return;//errhandler("CloseHandle", hwnd); 

    // Free memory.  
    GlobalFree((HGLOBAL)lpBits);
}

LONG WINAPI HandleException(struct _EXCEPTION_POINTERS* apExceptionInfo)
{
	MessageBox(NULL, "The application crashed :(\n\n"
		"Whitecake will save a dump and a screenshot if\n"
		"you enter a filename in the next dialog!", "Guru Meditation", 32);
	std::string path;
	if(NativeFileDialog(NULL, "dmp", "Dump-File", path, false)) {
		HANDLE hFile = CreateFileA(path.c_str(), GENERIC_WRITE, 
			FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
		ExInfo.ThreadId = ::GetCurrentThreadId();
		ExInfo.ExceptionPointers = apExceptionInfo;
		ExInfo.ClientPointers = false;

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, 
			MiniDumpWithFullMemory, &ExInfo, NULL, NULL);
		::CloseHandle(hFile);

		BITMAPINFO *bitmapInfo = CreateBitmapInfoStruct(hBitmap);
		if(bitmapInfo != NULL)
			CreateBMPFile((path + ".bmp").c_str(), bitmapInfo, hBitmap, hMemoryDC);

	}
	exit(1);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	InitCommonControls();

#ifndef _DEBUG
	SetUnhandledExceptionFilter(HandleException);   
#endif /* _DEBUG */


	MyRegisterClass(hInstance);
	if(!InitInstance(hInstance, nCmdShow))
		return(false);

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_WHITECAKE);

	while(GetMessage(&msg, NULL, 0, 0)) {
		if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// we delete the mainWindow-object just for debugging. In the release version
	// we're much faster without calling all those destructors! Just let Windows
	// free all memory for us
#ifdef _DEBUG
	delete mainWindow;
#endif /* _DEBUG */
	mainWindow = NULL;

	return((int) msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_WHITECAKE);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = (LPCTSTR) NULL; //IDC_WHITECAKE;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	hInst = hInstance;

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 800, 600, NULL, NULL, hInstance, NULL);

	if(!hWnd)
		return(false);

	RECT rect;
	GetClientRect(hWnd, &rect);
	mainWindow = new MainWindow(hWnd, rect.right, rect.bottom);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return(true);
}

bool IsVistaOrNewer()
{
	OSVERSIONINFO versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);

	GetVersionEx(&versionInfo);

	if(versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && versionInfo.dwMajorVersion >= 6)
		return(true);
	else
		return(false);
}

typedef HRESULT (__stdcall *DwmExtentFrameIntoClientAreaPtr)(HWND hWnd, const MARGINS *margins);
class DwmProxy
{
public:
	DwmProxy()
	{
		if(IsVistaOrNewer()) {
			dwmModule = LoadLibrary("Dwmapi.dll");
			extentFrameIntoClientAreaPtr = 
				(DwmExtentFrameIntoClientAreaPtr) GetProcAddress(dwmModule, "DwmExtendFrameIntoClientArea");
		} else {
			dwmModule = NULL;
			extentFrameIntoClientAreaPtr = NoExtentFrameIntoClientArea;
		}
	}

	inline bool HasDwm()
		{ return(dwmModule != NULL); };

	inline HRESULT ExtentFrameIntoClientArea(HWND hWnd, const MARGINS *margins)
		{ return(extentFrameIntoClientAreaPtr(hWnd, margins)); };

	static HRESULT __stdcall NoExtentFrameIntoClientArea(HWND hWnd, const MARGINS *margins)
		{ return(0); };

private:
	HMODULE dwmModule;
	DwmExtentFrameIntoClientAreaPtr extentFrameIntoClientAreaPtr;
};
DwmProxy dwmProxy;

int paintX = 0;
int paintY = 0;
int paintWidth = 0;
int paintHeight = 0;

// this window procedure contains all things relevant for borderless toplevel-windows
// it returns true if it can be processed in the WndProc and DefaultWndProc may be called
bool BorderlessProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT &result)
{
	if(message == WM_SIZE) {
		//SetWindowRgn(hWnd, NULL, FALSE); // prevent windows from using rounded corners at title

		if(wParam == SIZE_MAXIMIZED) {
			HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
			
			if(hMonitor) {
				MONITORINFO info;
				info.cbSize = sizeof(info);
				GetMonitorInfo(hMonitor, &info);
				int realWidth = abs(info.rcWork.right - info.rcWork.left);
				int realHeight = abs(info.rcWork.bottom - info.rcWork.top);

				int maybeWidth = LOWORD(lParam);
				int maybeHeight = HIWORD(lParam);

				paintX = (maybeWidth - realWidth) / 2;
				paintY = (maybeHeight - realHeight) / 2;
				paintWidth = realWidth;
				paintHeight = realHeight;
			}
		} else if(wParam == SIZE_RESTORED) {
			paintX = paintY = paintWidth = paintHeight = 0;
		}

		return(true);
	} else if(message == WM_CREATE && dwmProxy.HasDwm()) {
		SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
		const MARGINS shadow_on = { 1, 1, 1, 1 };
		dwmProxy.ExtentFrameIntoClientArea(hWnd, &shadow_on);
		result = 0;
		return(true);
	} else if(message == WM_CREATE && !dwmProxy.HasDwm()) {
		SetWindowTheme(hWnd, L"", L"");
		return(true);
	} else if(message == WM_NCACTIVATE) {
		result = TRUE;
		return(false);
	} else if(message == WM_NCPAINT && !dwmProxy.HasDwm()) {
		result = 0;
		return(false);
	} else if(message == WM_NCCALCSIZE) {
        result = 0; // overwrites WS_THICKFRAME, ...
		return(false);
	} else if(message == WM_NCHITTEST) {
		const LONG border_width = 8; //in pixels
		RECT windowRect;
		GetWindowRect(hWnd, &windowRect);
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);

		if(x >= windowRect.left && x < windowRect.left + border_width && 
			y < windowRect.bottom && y >= windowRect.bottom - border_width)
			result = HTBOTTOMLEFT;
		else if(x < windowRect.right && x >= windowRect.right - border_width && 
			y < windowRect.bottom && y >= windowRect.bottom - border_width)
			result = HTBOTTOMRIGHT;
		else if(x >= windowRect.left && x < windowRect.left + border_width &&
			y >= windowRect.top && y < windowRect.top + border_width)
			result = HTTOPLEFT;
		else if(x < windowRect.right && x >= windowRect.right - border_width &&
			y >= windowRect.top && y < windowRect.top + border_width)
			result = HTTOPRIGHT;
		else if(x >= windowRect.left && x < windowRect.left + border_width)
			result = HTLEFT;
		else if(x < windowRect.right && x >= windowRect.right - border_width)
			result = HTRIGHT;
		else if(y < windowRect.bottom && y >= windowRect.bottom - border_width)
			result = HTBOTTOM;
		else if(y >= windowRect.top && y < windowRect.top + border_width)
			result = HTTOP;
		else if(mainWindow->IsInTitle(x - windowRect.left, y - windowRect.top))
			result = HTCAPTION;
		else
			result = HTCLIENT;
		return(false);
	} else {
		return(true);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if(!BorderlessProc(hWnd, message, wParam, lParam, result))
		return(result);

	switch (message) 
	{
	case WM_ERASEBKGND:
		return(1);
	case WM_PAINT:
		OnPaint(hWnd);
		break;
	case WM_SIZE:
		// to work around that ugly behaviour of Windows (which makes maximized windows without
		// any frame much bigger than the screen is) we get the size from a special variable and
		// only use Windows' number if this is not available
		if(paintWidth == 0)
			mainWindow->OnResize(LOWORD(lParam), HIWORD(lParam));
		else
			mainWindow->OnResize(paintWidth, paintHeight);

		
		// very important, else we would not paint the additional area		
		recreateMemoryDC = true; 
		break;

	// TODO: If this window is not the toplevel item, find some other way to
	// get information about screen color-depth changes!!!
	case WM_DISPLAYCHANGE:
		recreateMemoryDC = true; 
		break;
	case WM_LBUTTONDOWN:
			mainWindow->OnMouseDown(GET_X_LPARAM(lParam) - paintX, 
				GET_Y_LPARAM(lParam) - paintY, wParam);
			mainWindowButtonState = 1;
		break;
	case WM_LBUTTONUP:
			if(mainWindowButtonState == 1) {
				mainWindow->OnMouseUp(GET_X_LPARAM(lParam) - paintX, 
					GET_Y_LPARAM(lParam) - paintY, wParam);
				mainWindowButtonState = 0;
			}
		break;
	case WM_LBUTTONDBLCLK:
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			mainWindow->OnMouseDoubleClick(GET_X_LPARAM(lParam) - paintX, 
				GET_Y_LPARAM(lParam) - paintY, wParam);
		}
		break;
	case WM_MOUSEMOVE:
			mainWindow->OnMouseMove(GET_X_LPARAM(lParam) - paintX, 
				GET_Y_LPARAM(lParam) - paintY, wParam);
		break;
	case WM_MOUSEWHEEL:
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			int height = rect.bottom;
			POINT point;
			point.x = GET_X_LPARAM(lParam);
			point.y = GET_Y_LPARAM(lParam);
			ScreenToClient(hWnd, &point);
			if(point.x < 0 || point.x > rect.right || 
				point.y < 0 || point.y > rect.bottom)
			{
				point.x = -1;
				point.y = height + 1;
			}

			mainWindow->OnMouseWheel(
				(double) GET_WHEEL_DELTA_WPARAM(wParam) / (double) WHEEL_DELTA,
				point.x, point.y, LOWORD(wParam));
		}
		break;
	case WM_KEYUP:
		mainWindow->OnKeyUp(wParam, 0);
		break;

	case WM_CLOSE:
		if(mainWindow->OnClose()) {
			DestroyWindow(hWnd);
		}
		return(0);

	case WM_DESTROY:
		DeleteObject(hBitmap);
		DeleteDC(hMemoryDC);
		PostQuitMessage(0);
		break;
	default:
		result = DefWindowProc(hWnd, message, wParam, lParam);
	}

	// if we have to redraw inform windows about this
	if(mainWindow && mainWindow->GetNeedRedraw()) {
		InvalidateRect(hWnd, NULL, false);
		mainWindow->SetNeedRedraw(false);
	}

	return(result);
}

void OnPaint(HWND hWnd)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	int width = paintWidth != 0 ? paintWidth : rect.right - rect.left;
	int height = paintHeight != 0 ? paintHeight : rect.bottom - rect.top;

	PAINTSTRUCT ps;
	HDC hdc;
	hdc = BeginPaint(hWnd, &ps);

	// to improve speed we just create a new memory DC only if size or color-mode
	// have changed since the last paint operation
	if(recreateMemoryDC) {
		// delete the old ones
		DeleteObject(hBitmap);
		DeleteDC(hMemoryDC);

		// create the new one with the new size...
		hMemoryDC = CreateCompatibleDC(hdc);
		hBitmap = CreateCompatibleBitmap(hdc, width, height);
		HBITMAP hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);
		DeleteObject(hOldBitmap); // delete this monchrome 1x1 pixel bitmap

		recreateMemoryDC = false;
	}

	Drawing drawing(hMemoryDC);
	mainWindow->OnPaint(&drawing);

	// do double buffering
	BitBlt(hdc, rect.left + paintX, rect.top + paintY, width, height, hMemoryDC, 0, 0, SRCCOPY);

	EndPaint(hWnd, &ps);
}
