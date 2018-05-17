#ifndef CHILDWINDOW_H
#define CHILDWINDOW_H

#include "Platform.h"

class ChildWindow
{ 
public:
	virtual void OnPaint(Drawing *drawing) = 0;
	virtual void OnResize(int width, int height) = 0;
	virtual void OnMouseDown(int x, int y, int modifiers) = 0;
	virtual void OnMouseMove(int x, int y, int modifiers) = 0;
	virtual void OnMouseUp(int x, int y, int modifiers) = 0;
	virtual void OnMouseDoubleClick(int x, int y, int modifiers) = 0;
	virtual void OnMouseWheel(double steps, int x, int y, int modifiers) = 0;
	virtual void OnKeyUp(int key, int modifiers) = 0;
	
	virtual ~ChildWindow() { };
};

#endif /* CHILDWINDOW_H */
