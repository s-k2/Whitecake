#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Platform.h"

class ChildWindow;
class ChartView;
class ToolBar;
class MenuBar;
class Project;
class Sub;

class MainWindow
{
public:
	MainWindow(NativeWindow nativeWindow, int width, int height);
	~MainWindow(void);

	// TODO: Compose the ChartView, the left ToolBar and the MenuBar to one image
	void OnPaint(Drawing *drawing);
	void OnResize(int width, int height);
	void OnMouseDown(int x, int y, int modifiers);
	void OnMouseMove(int x, int y, int modifiers);
	void OnMouseUp(int x, int y, int modifiers);
	void OnMouseDoubleClick(int x, int y, int modifiers);
	void OnMouseWheel(double steps, int x, int y, int modifiers);
	void OnKeyUp(int key, int modifiers);
	bool OnClose();

	inline NativeWindow GetNativeWindow()
		{ return(nativeWindow); };
	inline bool GetNeedRedraw()
		{ return(needRedraw); };
	inline void SetNeedRedraw(bool needRedraw)
		{ this->needRedraw = needRedraw; };
	inline int GetWidth()
		{ return(width); };
	inline int GetHeight()
		{ return(height); };

	inline Project *GetProject()
		{ return(project); };
	inline ChartView *GetChartView()
		{ return(chartView); };

	inline void SetChanged()
		{ hasChanged = true; };
	inline void ResetChanged()
		{ hasChanged = false; };
	inline bool HasChanged() const
		{ return(hasChanged); };

	bool AskForSave();
	void New();
	void Open();
	void Save();
	void SaveAs();
	void Send();
	void EditAliases();
	void ShowSettings();
	void Quit();

	
	inline bool IsInTitle(int x, int y)
		{ return(x > TabBarX && x < width - SystemButtonsWidth && y < TabBarY); };

private:
	Project *project;
	ChartView *chartView;
	NativeWindow nativeWindow;
	
	int width, height; // size of the mainwindow

	int toolBarY; // where is the toolbar? (toolbarX is alway zero)
	int firstTabIndex; // only used if the tab-bar is too small to show all
	int fullyVisibleTabsCount; // how many tabs can be shown?
	bool canShowAllTabs; // is the tab-bar big enough to show everything?

	static const int MenuBarItemWidth;
	static const int MenuBarHeight;
	static const int MenuBarWidth;
	static const int TabBarX;
	static const int TabBarY;
	static const int TabWidth;
	static const int TabButtonWidth; // size of the add, left and right buttons
	static const int TabPadding; // size of gap between tabs
	static const int TabBarHeight;
	static const int SystemButtonsWidth; // width of one of the minimize/maximize/close buttons
	static const int SystemButtonWidth; // width of all minimize/maximize/close buttons together
	static const int ToolHeight; // what's the size of a tool icon
	static const int ToolBarWidth;
	static const int ToolBarHeight;

	// if we changed something in the canvas we set this flag, to force the native
	// window to redraw the contents
	bool needRedraw;

	void PaintMenuBar(Drawing *drawing);
	void PaintTitleBar(Drawing *drawing);
	void PaintTabs(Drawing *drawing);
	void PaintToolBar(Drawing *drawing);

	// if a MouseDownEvent started in chartView, chartView must receive all 
	// mouse events to prevent funny things (especially when mouse-grabbing is
	// involved). To do this, we forward all following events (mouse move,
	// mouse up) to chartView
	bool forwardMouseEvents; 

	void OnMenuBarMouseUp(int x);
	void OnTabBarMouseUp(int x);
	void OnTitleBarMouseUp(int x);
	void OnTabBarDoubleClick(int x);
	void OnToolBarMouseUp(int y);

	void ShowSub(Sub *sub);

	inline bool IsInChartView(int x, int y)
		{ return(!(y <= MenuBarHeight && x <= MenuBarWidth) &&
			  !(y <= TabBarY + TabBarHeight && x > MenuBarWidth) &&
			  !(x <= ToolBarWidth && y >= toolBarY && y <= toolBarY + ToolBarHeight)); };

	// shows the main-menu and calls the appropriate handler for each item
	void ShowMainMenu();

	// has the project been changed since the last saving?
	bool hasChanged;

};

#endif /* MAINWINDOW_H */