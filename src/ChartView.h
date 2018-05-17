#ifndef CHART_VIEW
#define CHART_VIEW

#include "Sub.h"
#include "ChildWindow.h"
#include "MainWindow.h"
#include "Platform.h"
#include "Helper/Canvas.h"

class ChartItem;
struct InstructionInfo;

class ChartView : public ChildWindow
{
public:
	explicit ChartView(MainWindow *mainWindow);
	virtual ~ChartView();

	inline void SetSub(Sub *sub)
		{ this->sub = sub; };
	inline Sub *GetSub()
		{ return(sub); };

	void OnPaint(Drawing *drawing);
	inline void OnResize(int width, int height)
		{ CanvasSetViewSize(GetCanvas(), width, height); SetNeedRedraw(true); };
	void OnMouseDown(int x, int y, int modifiers);
	void OnMouseMove(int x, int y, int modifiers);
	void OnMouseUp(int x, int y, int modifiers);
	void OnMouseDoubleClick(int x, int y, int modifiers);
	void OnMouseWheel(double steps, int x, int y, int modifiers);
	void OnKeyUp(int key, int modifiers);

	enum Tool { PointerTool, BlockConnectorTool, IfBlockTool, InstructionTool, 
		StartBlockTool, EndBlockTool, AssignmentTool, CallSubTool, CommentTool };
	inline void SetCurrentTool(Tool tool)
		{ 
			currentTool = tool;
			if(currentTool == PointerTool)
				NativeSetPointerCursor(mainWindow->GetNativeWindow());
			else
				NativeSetCrossCursor(mainWindow->GetNativeWindow());
		};
	inline void SetCurrentTool(Tool tool, InstructionInfo *info)
		{ 
			currentTool = tool; currentInstructionInfo = info;
			if(currentTool == PointerTool)
				NativeSetPointerCursor(mainWindow->GetNativeWindow());
			else
				NativeSetCrossCursor(mainWindow->GetNativeWindow());
		};
	inline Tool GetCurrentTool()
		{ return(currentTool); };

	inline void SetNeedRedraw(bool needRedraw)
		{ mainWindow->SetNeedRedraw(needRedraw); };

	inline ChartItem *GetElementAt(int x, int y)
	{
		ChartItem *item = NULL;
		CanvasForeachItemAtViewPos(GetCanvas(), x, y, 
			GetElementAtHelper, &item);
		return(item);
	};

	static inline int AlignToGrid(int i) 
		{ return(i & 4 ? (i & ~7) + 8 : i & ~7); };

private:
	// some configuration
	static const int MinimumSelectionRectSize = 5;

	// the sub this view shows
	Sub *sub;
	
	MainWindow *mainWindow;

	// we use this enum to know what happens during the different calls of mouse
	// handler functions (e.g. MouseDown, MouseMove, MouseUp...)
	enum MouseState { MouseNoState, MouseDown, MouseDragging, MouseMoving };
	MouseState mouseState;

	// Where and on which item was the MouseDown-Event?
	int mouseDownX, mouseDownY;
	ChartItem *mouseDownItem;

	// If the user spans a rectangle to select item, drawSelectionRect is 
	// true and the following view-coordinates describe that rectangle
	bool drawSelectionRect;
	int selectionRectX, selectionRectY, selectionRectWidth, selectionRectHeight;

	// If the user started to move an item we update these numbers regularly
	int movingLastX;
	int movingLastY;

	// What tool is currently selected?
	Tool currentTool;
	// If the user selected the instruction tool, this points to the associated 
	// info-structure
	InstructionInfo *currentInstructionInfo;

	// these are callbacks of the canvas that do the actual work with each item
	static void PaintItem(Canvas *canvas, CanvasItem *item, void *userData);
	static void GetElementAtHelper(Canvas *canvas, CanvasItem *item, 
		void *userData, int x, int y);
	static void SelectionRectHelper(Canvas *canvas, 
		CanvasItem *item, void *userData);
	
	static void FindConnectorsAtInsert(Canvas *canvas, 
		CanvasItem *item, void *foundItem);

	void OnMouseUpPointer(int x, int y, int modfiers);

	// Get the canvas structure for the current visible sub
	inline Canvas *GetCanvas()
		{ return(sub->GetCanvas()); };
	inline double GetZoom()
		{ return(CanvasGetZoom(sub->GetCanvas())); };
};

#endif /* CHART_VIEW */
