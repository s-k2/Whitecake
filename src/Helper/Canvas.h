 /*
  *  Canvas.h - A simple and abstract canvas view
  *  Copyright (C) 2013  Stefan Klein
  *
  *  This library is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU Lesser General Public
  *  License version 2.1 as published by the Free Software Foundation
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  *
  */

#ifndef _CANVAS_H
#define _CANVAS_H

#include <stdlib.h>

struct CanvasItemS {
	int x, y;
	int width, height;
	void *itemData;
};
typedef struct CanvasItemS CanvasItem;

struct CanvasS {
	CanvasItem **items;
	size_t itemsCount;
	size_t itemsFree;
	
	// size of the view rectangle in screen coordinates
	//int viewX, viewY;
	int viewWidth, viewHeight;
	// size of the view rect in canvas coordinates
	double visibleX, visibleY;
	double visibleWidth, visibleHeight;

	double zoom;

	int minX, minY;
	int maxX, maxY;

	bool rearrangeItemsOnDelete;
};
typedef struct CanvasS Canvas;

extern void CanvasInit(Canvas *canvas);
extern void CanvasFree(Canvas *canvas); // just deletes all malloc'ed memory but not canvas itself

extern void CanvasItemAdd(Canvas *canvas, CanvasItem *canvasItem);
extern void CanvasItemUpdate(Canvas *canvas, CanvasItem *canvasItem);

// ATTENTION: Do not call insinde one of the ForeachFunctinos!
extern void CanvasItemRemove(Canvas *canvas, CanvasItem *canvasItem);

extern void CanvasSetViewSize(Canvas *canvas, int width, int height);
extern void CanvasSetViewOrigin(Canvas *canvas, int x, int y);
inline int CanvasGetViewWidth(Canvas *canvas)
	{ return(canvas->viewWidth); };
inline int CanvasGetViewHeight(Canvas *canvas)
	{ return(canvas->viewHeight); };

extern void CanvasSetZoom(Canvas *canvas, double zoom);
inline double CanvasGetZoom(Canvas *canvas)
{
	return(canvas->zoom);
};

extern void CanvasZoomWithSteadyPoint(Canvas *canvas, double newZoom,
									  int viewX, int viewY);

inline void CanvasViewToSurfacePosition(Canvas *canvas, 
										int viewX, int viewY, 
										int *surfaceX, int *surfaceY)
{
	*surfaceX = (int) (canvas->visibleX + viewX / canvas->zoom);
	*surfaceY = (int) (canvas->visibleY + viewY / canvas->zoom);
}

inline void CanvasExactViewToSurfacePosition(Canvas *canvas, int viewX, 
											 int viewY, double *surfaceX, 
											 double *surfaceY)
{
	*surfaceX = canvas->visibleX + viewX / canvas->zoom;
	*surfaceY = canvas->visibleY + viewY / canvas->zoom;
}

inline void CanvasSurfaceToViewPosition(Canvas *canvas, 
										int surfaceX, int surfaceY, 
										int *viewX, int *viewY)
{
	*viewX = (int) ((surfaceX - canvas->visibleX) * canvas->zoom);
	*viewY = (int) ((surfaceY - canvas->visibleY) * canvas->zoom);
}

inline void CanvasExactSurfaceToViewPosition(Canvas *canvas, 
											 int surfaceX, int surfaceY, 
											 double *viewX, double *viewY)
{
	*viewX = (surfaceX - canvas->visibleX) * canvas->zoom;
	*viewY = (surfaceY - canvas->visibleY) * canvas->zoom;
}

// be aware: these functions work ascending
extern void CanvasForeachItemInRect(Canvas *canvas, int x, int y,
									int width, int height, void (callback) 
									(Canvas *, CanvasItem *, void *), 
									void *userData);
inline void CanvasForeachVisibleItem(Canvas *canvas, void (callback) 
									 (Canvas *, CanvasItem *, void *), 
									 void *userData)
{
	CanvasForeachItemInRect(canvas, (int) canvas->visibleX, 
		(int) canvas->visibleY, (int) canvas->visibleWidth, 
		(int) canvas->visibleHeight, callback, userData);
};
extern void CanvasForeachItem(Canvas *canvas, void (callback)
						      (Canvas *, CanvasItem *, void *), 
							  void *userData);

// be aware: these functions work descending
extern void CanvasForeachItemAt(Canvas *canvas, 
								int canX, int canY, void (callback)
								(Canvas *, CanvasItem *, void *, int, int), 
								void *userData);
inline void CanvasForeachItemAtViewPos(Canvas *canvas, 
									   int viewX, int viewY, 
									   void (callback)(Canvas *, 
									   CanvasItem *, void *, int, int), 
									   void *userData)
{
	CanvasViewToSurfacePosition(canvas, viewX, viewY, &viewX, &viewY);
	CanvasForeachItemAt(canvas, viewX, viewY, callback, userData);
}

// calculates the sizes and positions of a scrollbar 
void CanvasGetScrollbarInfo(Canvas *canvas, double *xPos, double *xSize, 
	                        double *yPos, double *ySize);

inline void CanvasItemSetPos(Canvas *canvas, CanvasItem *item, int x, int y)
{
	item->x = x;
	item->y = y;

	if(x < canvas->minX)
		canvas->minX = x;
	if(y < canvas->minY)
		canvas->minY = y;
	if(x + item->width > canvas->maxX)
		canvas->maxX = x + item->width;
	if(y + item->height > canvas->maxY)
		canvas->maxY = y + item->height;
}

inline void CanvasItemSetPosRelative(Canvas *canvas, CanvasItem *item, int relX, int relY)
{
	CanvasItemSetPos(canvas, item, item->x + relX, item->y + relY);
}

inline void CanvasItemSetSize(Canvas *canvas, CanvasItem *item, int width, int height)
{
	item->width = width;
	item->height = height;
}

inline int CanvasItemGetX(CanvasItem *item)
{
	return(item->x);
}

inline int CanvasItemGetY(CanvasItem *item)
{
	return(item->y);
}

inline int CanvasItemGetWidth(CanvasItem *item)
{
	return(item->width);
}

inline int CanvasItemGetHeight(CanvasItem *item)
{
	return(item->height);
}


#endif /* CANVAS_H */
