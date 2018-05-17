#include "ChartView.h"

#include <string>
using std::string;

#include "Assignment.h"
#include "BlockConnector.h"
#include "Comment.h"
#include "IfBlock.h"
#include "Instruction.h"
#include "Platform.h"
#include "ProgramBlock.h"
#include "ProgramFlow.h"
#include "Project.h"

ChartView::ChartView(MainWindow *mainWindow)
{
	this->mainWindow = mainWindow;

	mouseState = MouseNoState;
	drawSelectionRect = false;

	currentTool = PointerTool;
}

ChartView::~ChartView()
{
}

void ChartView::PaintItem(Canvas *canvas, CanvasItem *item, void *userData)
{
	ChartItem *chartItem = (ChartItem *) item->itemData;
	register Drawing *drawing = (Drawing *) userData;
	double x, y;
	CanvasExactSurfaceToViewPosition(canvas, CanvasItemGetX(item), CanvasItemGetY(item), &x, &y);
		
	drawing->TranslateAndScale(x, y, CanvasGetZoom(canvas));
	
	int flags = 0;
	if(chartItem->GetSub()->HasSelection())
		flags |= chartItem->GetSub()->IsSelected(chartItem) ?
			ChartItem::PaintSelected : ChartItem::PaintNotSelected;
	if(chartItem->GetSub()->GetErrorItem() == chartItem)
		flags |= ChartItem::PaintErrorMark;

	chartItem->OnPaint(drawing, flags);

	drawing->RestoreTranslateAndScale();
}

void ChartView::OnPaint(Drawing *drawing)
{
	drawing->FillBackground(Stock::BackgroundGrey);

	CanvasForeachVisibleItem(GetCanvas(), PaintItem, drawing);

	// Draw the selection rectangle if needed
	if(drawSelectionRect)
		drawing->FillRect(selectionRectX, selectionRectY, selectionRectWidth, 
			selectionRectHeight, Stock::TransparentLightBlue);
	
	/*
	double widthBarX, widthBarSize, heightBarY, heightBarSize;
	CanvasGetScrollbarInfo(GetCanvas(), &widthBarX, &widthBarSize, &heightBarY, &heightBarSize);	
	
	double minX, minY, maxX, maxY;
	CanvasExactSurfaceToViewPosition(GetCanvas(), GetCanvas()->minX, GetCanvas()->minY, &minX, &minY);
	CanvasExactSurfaceToViewPosition(GetCanvas(), GetCanvas()->maxX, GetCanvas()->maxY, &maxX, &maxY);
	drawing->FillRect(minX, minY, maxX - minX, maxY - minY, Stock::TransparentLightBlue);

	static const int ScrollButtonSize = 24;
	int width = GetCanvas()->viewWidth;
	int height = GetCanvas()->viewHeight;
	drawing->FillRect(width - 16 - 4, 0, 16, height, Stock::LightGrey);
	drawing->FillRect(0, height - 16 - 4, width, 16, Stock::LightGrey);
	drawing->FillRect(width - 16 - 4, ScrollButtonSize + heightBarY * (height - 2 * ScrollButtonSize - 4), 16, 
		(height - 2 * ScrollButtonSize - 4) * heightBarSize, Stock::Grey);
	drawing->FillRect(widthBarX * width, height - 16 - 4, width * widthBarSize, 16, Stock::Grey);	*/
}


void ChartView::GetElementAtHelper(Canvas *canvas, CanvasItem *item, void *userData, int x, int y)
{
	if(*((ChartItem **) userData) == NULL && ((ChartItem *) item->itemData)->IsInItem(x, y))
		*((ChartItem **) userData) = (ChartItem *) item->itemData;
}

void ChartView::SelectionRectHelper(Canvas *canvas, CanvasItem *item, void *userData)
{
	register ChartView *view = (ChartView *) userData;
	register ChartItem *chartItem = (ChartItem *) item->itemData;
	
	if(chartItem->IsInItem(view->selectionRectX, view->selectionRectY, 
		view->selectionRectWidth, view->selectionRectHeight))
	{
		view->GetSub()->SelectItem(chartItem);
	}
}

void ChartView::OnMouseDown(int x, int y, int modifiers)
{
	// if there has been any error-information we clear it here
	GetSub()->SetErrorItem(NULL);

	// we just have to do any work when the pointer-tool is active
	if(currentTool == PointerTool) {
		mouseDownX = x;
		mouseDownY = y;
		mouseDownItem = GetElementAt(x, y);

		if(mouseDownItem) {
			CanvasViewToSurfacePosition(GetCanvas(), x, y, &x, &y);

			if(mouseDownItem->WantsDrag(x, y)) {
				// we can just drag one item, so we just select this item
				GetSub()->ClearSelection();
				GetSub()->SelectItem(mouseDownItem);
				mouseState = MouseDragging; // REDO: new(mouseDownItem, DRAG, oldDragData???)

				// we start grabing the mouse, because even if the user leaves the window
				// we want to be noticed about the movements and escpecially the release of
				// the mouse button...
				NativeGrabMouse(mainWindow->GetNativeWindow());
			} else {
				mouseState = MouseMoving; // REDO: new(mouseDownItem, MOVE, mouseDownItem->oldX, oldY)
				movingLastX = AlignToGrid(x);
				movingLastY = AlignToGrid(y);

				NativeGrabMouse(mainWindow->GetNativeWindow()); // see above

				// If this item has not been selected before
				if(!GetSub()->IsSelected(mouseDownItem)) {
					GetSub()->ClearSelection(); // remove the existing selection (if any)
					GetSub()->SelectItem(mouseDownItem); // and just select the item under the cursor
				}
			}
		} else { 
			// all other things (spanning of selection rect, single-selection, ...)
			// will be handled while moving when the button is released
			mouseState = MouseDown;

			NativeGrabMouse(mainWindow->GetNativeWindow());
		}
	}
}

void ChartView::OnMouseMove(int x, int y, int modifiers)
{
	// we just have to do any work when the pointer-tool is active
	if(currentTool == PointerTool) {
		switch(mouseState) {
		case MouseDragging:
			CanvasViewToSurfacePosition(GetCanvas(), x, y, &x, &y);
			x = AlignToGrid(x);
			y = AlignToGrid(y);
		
			// we can assert that there is exact one selected item (see above)
			GetSub()->GetSelectedItem(0)->OnDrag(x, y);
			SetNeedRedraw(true);
			break;

		case MouseMoving:
			{
				CanvasViewToSurfacePosition(GetCanvas(), x, y, &x, &y);
				x = AlignToGrid(x);
				y = AlignToGrid(y);

				// move all items that have been selected and that are independent... All
				// dependend items will be moved too, but this is done by its parent-items
				register int xDiff = x - movingLastX, yDiff = y - movingLastY;
				for(size_t i = 0; i < GetSub()->GetSelectionCount(); i++) {
					if(!GetSub()->GetSelectedItem(i)->IsDependent())
						GetSub()->GetSelectedItem(i)->Move(xDiff, yDiff);
				}
				movingLastX = x;
				movingLastY = y;
			}
			SetNeedRedraw(true);
			break;

		case MouseDown:
			{
				// just draw the selection rect if it is at least 5 pixel width/high
				register int diffX = x - mouseDownX, diffY = y - mouseDownY;
				drawSelectionRect = (diffX < -5 || diffX > 5) || (diffY < -5 || diffY > 5);
				selectionRectX = diffX > 0 ? mouseDownX : x;
				selectionRectY = diffY > 0 ? mouseDownY : y;
				selectionRectWidth = abs(diffX);
				selectionRectHeight = abs(diffY);
			}
			SetNeedRedraw(true);
			break;
			
		case MouseNoState:
		default:
			break;
		}
	} else { // all other tools are creating anything
		SetNeedRedraw(true);
	}
}

// I put this function just because of its size extra, no other reason
void ChartView::OnMouseUpPointer(int x, int y, int modfiers)
{
	switch(mouseState) {
	case MouseDragging:
	case MouseMoving:
		{
			ChartItem *item = GetElementAt(x, y);

			// if there has not been a real moving/dragging, just select the item
			register int diffX = x - mouseDownX, diffY = y - mouseDownY;
			if((diffX >= -5 && diffX <= 5) && (diffY >= -5 && diffY <= 5) &&
				item != NULL) 
			{
				GetSub()->ClearSelection();
				GetSub()->SelectItem(item);
				SetNeedRedraw(true);
			} else {
				SetNeedRedraw(true);
				mainWindow->SetChanged();
			}
		}
		NativeUngrabMouse(mainWindow->GetNativeWindow());
		break;

	case MouseDown:
		NativeUngrabMouse(mainWindow->GetNativeWindow());

		if(drawSelectionRect) {
			GetSub()->ClearSelection();

			register int diffX = x - mouseDownX, diffY = y - mouseDownY;
			selectionRectX = diffX > 0 ? mouseDownX : x;
			selectionRectY = diffY > 0 ? mouseDownY : y;
			selectionRectWidth = abs(diffX);
			selectionRectHeight = abs(diffY);
			
			CanvasViewToSurfacePosition(GetCanvas(), 
				selectionRectX, selectionRectY, 
				&selectionRectX, &selectionRectY);
			// we make the assumption that distances can be scaled using
			// GetZoom() * screen distance
			selectionRectWidth = (int) ((double) selectionRectWidth / GetZoom());
			selectionRectHeight = (int) ((double) selectionRectHeight / GetZoom());

			CanvasForeachItemInRect(GetCanvas(), 
				selectionRectX, selectionRectY, 
				selectionRectWidth, selectionRectHeight, 
				SelectionRectHelper, this);

			drawSelectionRect = false;
			SetNeedRedraw(true);
		} else {
			ChartItem *item = GetElementAt(x, y);
			if(item) { // single-selection
				GetSub()->ClearSelection();
				GetSub()->SelectItem(item);
				SetNeedRedraw(true);
			} else if(GetSub()->HasSelection()) {
				GetSub()->ClearSelection();
				SetNeedRedraw(true);
			}
		}
		break;
		
		case MouseNoState:
		default:
			break;
	}
	mouseState = MouseNoState;
}

struct FindConnectorsInfo
{
	ChartItem *newItem;
	BlockConnector *foundConnector;
};

void ChartView::FindConnectorsAtInsert(Canvas *canvas, 
		CanvasItem *item, void *infoVoid)
{
	FindConnectorsInfo *info = (FindConnectorsInfo *) infoVoid;
	if(info->foundConnector != NULL)
		return;

	BlockConnector *blockConnector = dynamic_cast<BlockConnector *>((ChartItem *) item->itemData);
	if(blockConnector != NULL && blockConnector->GetEndItem() == NULL && 
			info->newItem->IsInItem(blockConnector->GetEndX(), blockConnector->GetEndY()))
	{
		info->foundConnector = blockConnector;
	}
}

void ChartView::OnMouseUp(int x, int y, int modifiers)
{
	if(currentTool == PointerTool) {
		OnMouseUpPointer(x, y, modifiers);
	} else {
		GetSub()->ClearSelection();
		CanvasViewToSurfacePosition(GetCanvas(), x, y, &x, &y);
		ChartItem *newItem;

		switch(currentTool) {
		case StartBlockTool:
			newItem = new StartBlock(GetSub(), 
				AlignToGrid(x - StartBlock::Width / 2), 
				AlignToGrid(y - StartBlock::Height / 2)); // REDO: new(newItem, NEW_ITEM)
			break;
		case IfBlockTool:
			newItem = new IfBlock(GetSub(), 
				AlignToGrid(x - IfBlock::Width / 2), 
				AlignToGrid(y - IfBlock::Height / 2)); // REDO: new(newItem, NEW_ITEM)
			break;
		case InstructionTool:
			newItem = new Instruction(GetSub(), 
				AlignToGrid(x - Instruction::Width / 2), 
				AlignToGrid(y - Instruction::Height / 2), 
				currentInstructionInfo); // REDO: new(newItem, NEW_ITEM)
			break;
		case EndBlockTool:
			newItem = new EndBlock(GetSub(), 
				AlignToGrid(x - EndBlock::Width / 2), 
				AlignToGrid(y - EndBlock::Height / 2)); // REDO: new(newItem, NEW_ITEM)
			break;
		case CallSubTool:
			newItem = new CallSub(GetSub(), 
				AlignToGrid(x - CallSub::Width / 2), 
				AlignToGrid(y - CallSub::Height / 2)); // REDO: new(newItem, NEW_ITEM)
			break;
		case CommentTool:
			newItem = new Comment(GetSub(), AlignToGrid(x - CallSub::Width / 2), 
				AlignToGrid(y - CallSub::Height / 2));
			break;
		case AssignmentTool:
			newItem = new Assignment(GetSub(), 
				AlignToGrid(x - Assignment::Width / 2), 
				AlignToGrid(y - Assignment::Height / 2)); // REDO: new(newItem, NEW_ITEM)
			break;
		default:
			return;
		}
		GetSub()->SelectItem(newItem);
		if(newItem->HasConnectionPoint()) {
			struct FindConnectorsInfo info = { newItem, NULL };
			CanvasForeachItemInRect(GetCanvas(), newItem->GetX(), newItem->GetY(), 
				newItem->GetWidth(), newItem->GetHeight(), FindConnectorsAtInsert, 
				&info);
			// connecto if something was found and it's not the item we create
			if(info.foundConnector && !newItem->HasDepedendent(info.foundConnector))
				info.foundConnector->ConnectTo(newItem);
		}

		SetCurrentTool(PointerTool);
		mainWindow->SetChanged();
		SetNeedRedraw(true);
	}

}

void ChartView::OnMouseDoubleClick(int x, int y, int modifiers)
{
	// be aware that before all DoubleClick-Events MouseUp has already been called

	if(currentTool == PointerTool) {
	
		// although mouseState is MouseNoState the mouseDown-coordinates are correct
		// because before any OnMouseDoubleClick-call OnMouseDown has been called

		// if there is an element under the cursor and the mouse has not 
		// moved, we can assume that the user wants to double-click and
		// change it

		ChartItem *item = GetElementAt(x, y);

		// if there has not been a real moving/dragging, this was a real double-click
		register int diffX = x - mouseDownX, diffY = y - mouseDownY;
		if((diffX >= -5 && diffX <= 5) && (diffY >= -5 && diffY <= 5) &&
			item != NULL)
		{
			if(item->OnEdit(mainWindow->GetNativeWindow())) {
					mainWindow->SetChanged();
					SetNeedRedraw(true);
			}
		}
	}
}

void ChartView::OnMouseWheel(double steps, int x, int y, int modifiers)
{
	if(modifiers & MODIFIER_CTRL) {
		double newZoom = GetZoom() + steps * 0.1;
		if(newZoom < 0.1)
			newZoom = 0.1;

		CanvasZoomWithSteadyPoint(GetCanvas(), newZoom, x, y);
	} else if(modifiers & MODIFIER_SHIFT) {
		GetCanvas()->visibleX -= (GetCanvas()->visibleWidth / 20) * steps;
	} else {
		GetCanvas()->visibleY -= (GetCanvas()->visibleHeight / 20) * steps;
	}
	SetNeedRedraw(true);
}

void ChartView::OnKeyUp(int key, int modifiers)
{
	if(key == KEY_DEL) {
		// With multiple selections it is a problem to delete the independend
		// objects before the dependend objects. To solve this problem we:
		//  ...delete all independent objects at first
		//  ...dependent objects will be deleted then - 
		//     if not: the bug must be located somewhere else
		if(GetSub()->HasSelection()) {
			bool hasDeletedAnything = false;

			for(size_t i = 0; i < GetSub()->GetSelectionCount(); i++) {
				register ChartItem *current = GetSub()->GetSelectedItem(i);
				// neither dependend items nor the start item may be deleted
				if(!current->IsDependent() && 
					current != GetSub()->GetStartBlock()) 
				{
					delete current; // REDO: new(item, ITEM_DELETE, currentState?)
					hasDeletedAnything = true;
				}
			}

			// clear the selection and redraw only if something was removed
			if(hasDeletedAnything) { 
				GetSub()->ClearSelection();
				mainWindow->SetChanged();
				SetNeedRedraw(true);
			}
		}
	}
}
