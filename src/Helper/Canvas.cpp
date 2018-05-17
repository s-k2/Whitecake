 /*
  *  Canvas.c - A simple and abstract canvas view
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

#include "Canvas.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

inline int minInt(int x, int y)
{
	return(x < y ? x : y);
}

inline int maxInt(int x, int y)
{
	return(x > y ? x : y);
}

inline double minDouble(double x, double y)
{
	return(x < y ? x : y);
}

inline double maxDouble(double x, double y)
{
	return(x > y ? x : y);
}

int IntersectRectangles(int x1, int y1, int w1, int h1,
						int x2, int y2, int w2, int h2)
{
	int cl = minInt(x1, x2); // circumscribed rectangle's left-value
	int cb = minInt(y1, y2); // ... and the bottom-value
	int cr = maxInt(x1 + w1, x2 + w2); // ... right-value
	int ct = maxInt(y1 + h1, y2 + h2); // ... top-value

	return(w1 + w2 > cr - cl && h1 + h2 > ct - cb);
}

void CanvasInit(Canvas *canvas)
{
	canvas->items = NULL;
	canvas->itemsCount = 0;
	canvas->itemsFree = 0;
	canvas->zoom = 1.0;
	canvas->minX = canvas->minY = canvas->maxX = canvas->maxY = 0;
	canvas->rearrangeItemsOnDelete = true;
}

void CanvasFree(Canvas *canvas)
{
	// TODO: Check if all items have been deleted???
	free(canvas->items);
	canvas->items = NULL;
	canvas->itemsCount = 0;
	canvas->itemsFree = 0;
}

void CanvasItemAdd(Canvas *canvas, CanvasItem *canvasItem)
{
	static const size_t ITEM_BLOCKSIZE = 256;
	
	if(canvas->itemsFree > 0) {
		canvas->items[canvas->itemsCount] = canvasItem;
		canvas->itemsCount++;
		canvas->itemsFree--;
	} else {
		canvas->items = (CanvasItem **) realloc(canvas->items, 
			sizeof(CanvasItem *) * (canvas->itemsCount + ITEM_BLOCKSIZE));
		canvas->items[canvas->itemsCount] = canvasItem;
		canvas->itemsCount++;
		canvas->itemsFree = ITEM_BLOCKSIZE - 1;
	}
}

void CanvasItemUpdate(Canvas *canvas, CanvasItem *item)
{
	// Is there anything to do? I think not!
	//  Maybe we can update the zLevel... but it's time-consuming!!!
}

void CanvasItemRemove(Canvas *canvas, CanvasItem *item)
{
	size_t i;

	// Why not save the zLevel in each item to speed up looking for the item?
	// We must allocate more memory for each item! And, even if we would save 
	// the zLevel within each item we would have to run a loop to update all 
	// other zLevel entries... 
	// So we choose a bad way - but it could be even worse ;-)
	for(i = 0; i < canvas->itemsCount; i++) {
		if(canvas->items[i] == item) {
			// reorder the items-array if allowed
			if(canvas->rearrangeItemsOnDelete) {
				canvas->itemsFree++;
				canvas->itemsCount--;
				memmove(canvas->items + i, canvas->items + i + 1, 
					(canvas->itemsCount - i) * sizeof(size_t));
			} else { // else just but a null pointer
				canvas->items[i] = NULL;
			}
		}
	}
}

void CanvasRearrangeItems(Canvas *canvas)
{
	size_t i, lastRearrangedIndex;
	for(i = 0, lastRearrangedIndex = 0; i < canvas->itemsCount; i++) {
		if(canvas->items[i] != NULL) {
			canvas->items[lastRearrangedIndex] = canvas->items[i];
			lastRearrangedIndex++;
		}
	}

	canvas->itemsFree = canvas->itemsCount + canvas->itemsFree - lastRearrangedIndex;
	canvas->itemsCount = lastRearrangedIndex;
}

void CanvasSetViewSize(Canvas *canvas, int width, int height)
{
	canvas->viewWidth = width;
	canvas->viewHeight = height;

	canvas->visibleWidth = width / canvas->zoom;
	canvas->visibleHeight = height / canvas->zoom;
}

void CanvasSetViewOrigin(Canvas *canvas, int x, int y)
{
	canvas->visibleX = x;
	canvas->visibleY = y;
}

void CanvasSetZoom(Canvas *canvas, double zoom)
{
	canvas->zoom = zoom;

	// now the area of the visible items has changed
	canvas->visibleWidth = (canvas->viewWidth / zoom);
	canvas->visibleHeight = (canvas->viewHeight / zoom);
}

void CanvasZoomWithSteadyPoint(Canvas *canvas, double newZoom,
							   int viewX, int viewY)
{
	double surfaceSteadyX, surfaceSteadyY;
	CanvasExactViewToSurfacePosition(canvas, viewX, viewY, 
		&surfaceSteadyX, &surfaceSteadyY);
	
	CanvasSetZoom(canvas, newZoom);
	canvas->visibleX = surfaceSteadyX - viewX / newZoom;
	canvas->visibleY = surfaceSteadyY - viewY / newZoom;
}

void CanvasForeachItemInRect(Canvas *canvas, 
							 int x, int y, int width, int height, 
							 void (callback) (Canvas *, CanvasItem *, void *), 
							 void *userData)
{
	bool oldRearrangeItemsValue = canvas->rearrangeItemsOnDelete;
	canvas->rearrangeItemsOnDelete = false;

	size_t i;
	for(i = 0; i < canvas->itemsCount; i++) {
		if(canvas->items[i] != NULL && IntersectRectangles(x, y, width, height, 
			canvas->items[i]->x, canvas->items[i]->y,
			canvas->items[i]->width, canvas->items[i]->height))
		{
			callback(canvas, canvas->items[i], userData);
		}
	}

	if(oldRearrangeItemsValue == true)
		CanvasRearrangeItems(canvas);
	canvas->rearrangeItemsOnDelete = oldRearrangeItemsValue;
}

extern void CanvasForeachItem(Canvas *canvas, void (callback)
						      (Canvas *, CanvasItem *, void *), 
							  void *userData)
{
	bool oldRearrangeItemsValue = canvas->rearrangeItemsOnDelete;
	canvas->rearrangeItemsOnDelete = false;

	size_t i;
	for(i = 0; i < canvas->itemsCount; i++) {
		if(canvas->items[i] != NULL)
			callback(canvas, canvas->items[i], userData);
	}

	if(oldRearrangeItemsValue == true)
		CanvasRearrangeItems(canvas);
	canvas->rearrangeItemsOnDelete = oldRearrangeItemsValue;
}

void CanvasForeachItemAt(Canvas *canvas, 
						 int x, int y, void (callback) 
						 (Canvas *, CanvasItem *, void *, int, int), 
						 void *userData)
{
	bool oldRearrangeItemsValue = canvas->rearrangeItemsOnDelete;
	canvas->rearrangeItemsOnDelete = false;

	size_t i;
	for(i = canvas->itemsCount - 1; i != (size_t) -1; i--) {
		if(canvas->items[i] != NULL && 
			canvas->items[i]->x <= x && 
			x <= canvas->items[i]->x + canvas->items[i]->width &&
			canvas->items[i]->y <= y && 
			y <= canvas->items[i]->y + canvas->items[i]->height)
		{
			callback(canvas, canvas->items[i], userData, x, y);
		}
	}

	if(oldRearrangeItemsValue == true)
		CanvasRearrangeItems(canvas);
	canvas->rearrangeItemsOnDelete = oldRearrangeItemsValue;
}


// calculates the sizes and positions of a scrollbar 
void CanvasGetScrollbarInfo(Canvas *canvas, double *xPos, double *xSize, 
	                        double *yPos, double *ySize)
{
	double totalWidth = canvas->maxX - canvas->minX;
	// size is easy...
	*xSize = minDouble(canvas->visibleWidth / totalWidth, 1.0);
	// but the position is a bit more tricky...
	// With the overall min-call we prevent xPos from getting bigger than the
	// second argument of that min - which is the total size minus the size
	// of on page (but not less than 0 in case on page is bigger than the
	// maximum position). The first argument to the minimum-function is the
	// code-path taken in hardly all cases... it just returns the distance of
	// currentX and minX and takes sure that it is not less than 0 (if currentX
	// is higher than minX)
	*xPos = minDouble(
		maxDouble(canvas->visibleX - canvas->minX, 0),
		maxDouble(canvas->maxX - canvas->visibleWidth, 0)) / totalWidth;

	// in y direction the same as above happens... so nothing more to say
	double totalHeight = canvas->maxY - canvas->minY;
	*ySize = minDouble(canvas->visibleHeight / totalHeight, 1.0);
	*yPos = minDouble(
		maxDouble(canvas->visibleY - canvas->minY, 0), 
		maxDouble(canvas->maxY - canvas->visibleHeight, 0)) / totalHeight;
}