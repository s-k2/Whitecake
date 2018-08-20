#include "BlockConnector.h"

#include <math.h>
#include <algorithm>
#include <vector>
using std::max;
using std::min;
using std::vector;

#include "Sub.h"
#include "CodeWriteException.h"
#include "Compiler.h"
#include "Helper/XMLReader.h"
#include "Helper/XMLWriter.h"
#include "Translate.h"

const int BlockConnector::Width = 0;
const int BlockConnector::Height = 64;

// used as buffer sizes for the lines and selection
static const int BufferSize = 5;
static const int ExtraBuffer = 25;
static const int ThinExtraBufferForDowns = 2; // due to the pen-thickness we need a bit of extra-space

BlockConnector::BlockConnector(Sub *sub, int startX, int startY, bool toRight)
	: ChartItem(sub, startX, startY, Width, Height), 
		toRight(toRight), partsDirection(toRight ? Direction::Right : Direction::Down)
{
	dependentItems[0] = dependentItems[1] = NULL;
	isDependent = true;

	endItem = NULL;
	this->startX = startX;
	this->startY = startY;
	SetEndPoint(startX + (toRight ? Height : 0), startY + Height);
}

BlockConnector::BlockConnector(Sub *sub)
	: ChartItem(sub, Width, Height), partsDirection(Direction::Down)
{
	dependentItems[0] = dependentItems[1] = NULL;
	isDependent = true;

	endItem = NULL;
	startX = startY = 0;
	SetEndPoint(startX + Width, startY + Height);

	draggingEndPoint = false;
}

BlockConnector::~BlockConnector(void)
{
	if(endItem)
		endItem->InputConnectorRemove(this);
}

void BlockConnector::Move(int relX, int relY)
{
	startX += relX;
	startY += relY;

	// move the end-position only if it's not already connected
	if(endItem == NULL) {
		endX += relX;
		endY += relY;
	}

	DetermineParts();
	UpdateCanvasItem();
}

bool IsNear(int x1, int y1, int x2, int y2)
{
	return((x1 >= x2 - BufferSize && x1 <= x2 + BufferSize &&
		y1 >= y2 - 2 * BufferSize && y1 <= y2 + BufferSize));
}

bool BlockConnector::WantsDrag(int x, int y)
{
	if(IsNear(x, y, endX, endY)) {
		draggingEndPoint = true;
		return(true);
	} else if((movedPart = GetPartAt(x, y)) != NULL) {
		if(movedPart == &parts.front() || movedPart == &parts.back()) // don't move first an last line
			return(false);
		draggingEndPoint = false;
		movePartInitialX = x;
		movePartInitialY = y;
		return(true);
	}
	return(false);
}

void BlockConnector::OnDrag(int x, int y)
{
	if(draggingEndPoint)
		AdjustEndToPlugable(x, y);
	else {
		if(movedPart->GetDirection().IsHorizontal())
			movedPart->SetMoveBy(movedPart->GetMoveBy() + (y - movePartInitialY));
		else
			movedPart->SetMoveBy(movedPart->GetMoveBy() + (x - movePartInitialX));
		UpdateCanvasItem();

		movePartInitialX = x;
		movePartInitialY = y;
	}
}

void BlockConnector::ConnectTo(ChartItem *item)
{
	// if there has been an old endItem, disconnect from it
	endItem = item;
	item->InputConnectorAdd(this);
	SetEndPoint(item->GetX() + item->GetWidth() / 2, item->GetY());
}

void BlockConnector::UpdateMovedEndItem()
{
	SetEndPoint(endItem->GetX() + endItem->GetWidth() / 2, endItem->GetY());
}

void BlockConnector::AdjustEndToPlugable(int absX, int absY)
{
	// Is there an item at the end of this connector?
	ChartItem *newItemAtEnd = NULL;
	CanvasForeachItemAt(GetSub()->GetCanvas(), absX, absY, FindPlugableItem, &newItemAtEnd);
	
	// disconnect from the old item
	if(endItem)
		endItem->InputConnectorRemove(this);

	// if there is a new item, connect to it
	if(newItemAtEnd) {
		// if there has been an old endItem, disconnect from it
		newItemAtEnd->InputConnectorAdd(this);
		absX = newItemAtEnd->GetX() + newItemAtEnd->GetWidth() / 2;
		absY = newItemAtEnd->GetY();
	}

	endItem = newItemAtEnd; // even it's NULL

	SetEndPoint(absX, absY);
}

void BlockConnector::FindPlugableItem(Canvas *canvas, CanvasItem *item, 
		void *userData, int x, int y)
{
	register ChartItem *chartItem = (ChartItem *) item->itemData;
	register ChartItem **foundItem = (ChartItem **) userData;

	if(chartItem->HasConnectionPoint() && chartItem->IsInItem(x, y))
	{
		*foundItem = chartItem;
	}
}

void BlockConnector::SetEndPoint(int absX, int absY)
{
	endX = absX;
	endY = absY;

	DetermineParts();
	UpdateCanvasItem();
}

void BlockConnector::DetermineParts()
{
	std::vector<ConnectorPart> newParts;

	static const int WrongDirectionSize = 32;
	static const int AroundBlockSize = 96;

	Direction overallDir(startX, startY, endX, endY);
	if(!toRight) {
		if(!overallDir.IsLeft() && !overallDir.IsRight()) {
			if(overallDir.IsDown())
				newParts.push_back(ConnectorPart(Direction(Direction::Down)));
			else {
				newParts.push_back(ConnectorPart(Direction(Direction::Down), 0));
				newParts.push_back(ConnectorPart(Direction(Direction::Left), WrongDirectionSize));
				newParts.push_back(ConnectorPart(Direction(Direction::Up), AroundBlockSize));
				newParts.push_back(ConnectorPart(Direction(Direction::Right), -WrongDirectionSize));
				newParts.push_back(ConnectorPart(Direction(Direction::Down), 0));
			}
		} else {
			if(overallDir.IsDown()) {
				newParts.push_back(ConnectorPart(Direction(Direction::Down)));
				newParts.push_back(ConnectorPart(overallDir.GetOnlyHorizontal()));
				newParts.push_back(ConnectorPart(Direction(Direction::Down)));
			} else if(overallDir.IsUp()) {
				newParts.push_back(ConnectorPart(Direction(Direction::Down)));
				newParts.push_back(ConnectorPart(overallDir.GetOnlyHorizontal(), WrongDirectionSize));
				newParts.push_back(ConnectorPart(Direction(Direction::Up)));
				newParts.push_back(ConnectorPart(overallDir.GetOnlyHorizontal(), -WrongDirectionSize));
				newParts.push_back(ConnectorPart(Direction(Direction::Down)));
			}
		}
	} else {
		if(overallDir.IsRight()) {
			newParts.push_back(ConnectorPart(Direction(Direction::Right)));
			if(overallDir.IsDown())
				newParts.push_back(ConnectorPart(Direction(Direction::Down)));
			else {
				newParts.push_back(ConnectorPart(Direction(Direction::Up)));
				newParts.push_back(ConnectorPart(Direction(Direction::Right), -WrongDirectionSize));
				newParts.push_back(ConnectorPart(Direction(Direction::Down)));
			}
		} else {
			newParts.push_back(ConnectorPart(Direction(Direction::Right)));
			newParts.push_back(ConnectorPart(overallDir.GetOnlyVertical(), WrongDirectionSize));
			newParts.push_back(ConnectorPart(Direction(Direction::Left), -WrongDirectionSize));
			newParts.push_back(ConnectorPart(Direction(Direction::Down)));
		}
	}

	// if the direction stays the same, preserve the moveBy's
	if(overallDir == partsDirection) {
		for(auto oldIt = parts.begin(), newIt = newParts.begin(); oldIt != parts.end() && newIt != newParts.end(); ++oldIt, ++newIt) {
			newIt->SetMoveBy(oldIt->GetMoveBy());
		}
	}
	parts = newParts;
	partsDirection = overallDir;
}

class PartsIterator
{
public:
	PartsIterator(vector<ConnectorPart> &parts ,int startX, int startY, int endX, int endY)
		: overallDir(startX, startY, endX, endY), 
			totalWidth(endX - startX), totalHeight(endY - startY), 
			it(parts.begin()), itBegin(parts.begin()), itEnd(parts.end()),
			curX(startX), curY(startY),
			lastX(startX), lastY(startY)
	{
		horizontalInDirectionCount = GetCountOf<Direction::Horizontal>(parts, overallDir);
		verticalInDirectionCount = GetCountOf<Direction::Vertical>(parts, overallDir);

		UpdateCurPos();
	}

	int GetLastX()
		{ return(lastX); };
	int GetLastY()
		{ return(lastY); };
	int GetCurX()
		{ return(curX); };
	int GetCurY()
		{ return(curY); };
	ConnectorPart *GetCurPart()
		{ return(&(*it)); };

	bool ToNext() {
		++it;
		if(it == itEnd)
			return(false);
		UpdateCurPos();
		return(true);
	}

private:
	template<int Search>
	int GetCountOf(const vector<ConnectorPart> &parts, Direction searchedDirection)
	{
		int count = 0;
		for(auto it = parts.begin(); it != parts.end(); ++it)
			if(it->GetDirection().GetOnly<Search>() == searchedDirection.GetOnly<Search>())
				count++;
		return(count);
	}

	void UpdateCurPos() {
		lastX = curX;
		lastY = curY;
		if(it->GetDirection().IsHorizontal()) {
			if(it->GetDirection().GetOnlyHorizontal() == overallDir.GetOnlyHorizontal())
				curX += totalWidth / horizontalInDirectionCount;
			if(it != itBegin) {
				auto prev = std::prev(it);
				if(prev->GetMoveBy() != 0) {
					curX -= prev->GetMoveBy();
				}
			}
			auto next = it + 1;
			if(next != itEnd && next->GetMoveBy() != 0) {
				curX += next->GetMoveBy();
			}
		} else {
			if(it->GetDirection().GetOnlyVertical() == overallDir.GetOnlyVertical())
				curY += totalHeight / verticalInDirectionCount;
			if(it != itBegin) {
				auto prev = std::prev(it);
				if(prev->GetMoveBy() != 0) {
					curY -= prev->GetMoveBy();
				}
			}
			auto next = it + 1;
			if(next != itEnd && next->GetMoveBy() != 0) {
				curY += next->GetMoveBy();
			}
		}
	}
	Direction overallDir;
	int totalWidth, totalHeight;
	vector<ConnectorPart>::iterator it;
	vector<ConnectorPart>::iterator itBegin;
	vector<ConnectorPart>::iterator itEnd;
	int curX, curY;
	int lastX, lastY;

	int horizontalInDirectionCount;
	int verticalInDirectionCount;
};

void BlockConnector::UpdateCanvasItem()
{
	int leftCanvasPos = min(startX, endX);
	int topCanvasPos = min(startY, endY);
	int rightCanvasPos = max(startX, endX);
	int bottomCanvasPos = max(startY, endY);
	
	PartsIterator it(parts, startX, startY, endX, endY);
	do {
		leftCanvasPos = min(it.GetCurX(), leftCanvasPos);
		topCanvasPos = min(it.GetCurY(), topCanvasPos);
		rightCanvasPos = max(it.GetCurX(), rightCanvasPos);
		bottomCanvasPos = max(it.GetCurY(), bottomCanvasPos);
	} while(it.ToNext());

	CanvasItemSetPos(GetSub()->GetCanvas(), &item, leftCanvasPos - BufferSize, topCanvasPos - BufferSize);
	CanvasItemSetSize(GetSub()->GetCanvas(), &item, 
		rightCanvasPos - leftCanvasPos + 2 * BufferSize, 
		bottomCanvasPos - topCanvasPos + 2 * BufferSize);
}

void BlockConnector::OnPaint(Drawing *drawing, int flags)
{
	Pen *pen;
	Color *color;
	if(flags & PaintNotSelected) { // paint it light if this connector is not part of the current selection
		pen = &Stock::ThickerLighterGreyPen;
		color = &Stock::LighterGrey;
	} else if(flags & PaintSelected) {
		pen = &Stock::ThickerCrimsonPen;
		color = &Stock::Crimson;
	} else { // if there is no selection...
		if(endItem) { // paint it black if it has a connection
			pen = &Stock::ThickerBlackPen;
			color = &Stock::Black;
		} else { // or not as light as if unselected if it has no connection
			pen = &Stock::ThickerDarkGreyPen;
			color = &Stock::DarkGrey;
		}
	}

	//drawing->FillRect(0, 0, GetWidth(), GetHeight(), Stock::TransparentLightBlue);
	drawing->TranslateAndScale(- GetX(), - GetY(), 1.0);

	PartsIterator it(parts, startX, startY, endX, endY);

	Path path;
	path.MoveTo(startX, startY);
	do {
		path.LineTo(it.GetCurX(), it.GetCurY());
	} while(it.ToNext());
	drawing->DrawPath(path, *pen);

	if(endItem) { // draw a filled triangle
		drawing->FillTriangle(it.GetCurX() - 7, it.GetCurY() - 8, 
			it.GetCurX() + 7, it.GetCurY() - 8, it.GetCurX(), it.GetCurY(), *color);
	} else { // draw two crossing lines
		drawing->DrawLine(it.GetCurX() - 5, it.GetCurY() - 5, it.GetCurX() + 5, it.GetCurY() + 5, *pen);
		drawing->DrawLine(it.GetCurX() - 5, it.GetCurY() + 5, it.GetCurX() + 5, it.GetCurY() - 5, *pen);
	}

	if(flags & PaintErrorMark)
		drawing->DrawEllipse(it.GetCurX() - 15, it.GetCurY() - 15, 30, 30, Stock::ThickerCrimsonPen);

	drawing->RestoreTranslateAndScale();
}

ConnectorPart *BlockConnector::GetPartAt(int x, int y)
{
	// To do the check we make a surrounding rect for all line segments of this
	// connector. These surrounding rects are bigger than the line itself to
	// give the user a better chance to hit the line.

	int rl, rr, rb, rt; // the rect of the current line segment
	
	PartsIterator it(parts, startX, startY, endX, endY);
	do {
		rl = min(it.GetLastX(), it.GetCurX()) - BufferSize;
		rr = max(it.GetLastX(), it.GetCurX()) + BufferSize;
		rb = min(it.GetLastY(), it.GetCurY()) - BufferSize;
		rt = max(it.GetLastY(), it.GetCurY()) + BufferSize;

		if(rl <= x && rr >= x && rb <= y && rt >= y)
			return(it.GetCurPart());
	} while(it.ToNext());

	return(NULL);
}

bool BlockConnector::IsInItem(int x, int y)
{
	return(GetPartAt(x, y) != NULL);
}

bool BlockConnector::IsInItem(int x, int y, int width, int height)
{
	// We do the same as above here: For every segment create a surrounding 
	// rect and check if the rect supplied by this functions argument hits it

	int rl, rr, rb, rt; // the rect of the current line segment

	PartsIterator it(parts, startX, startY, endX, endY);
	do {
		// make the surrounding rect for the current line segment
		rl = min(it.GetLastX(), it.GetCurX()) - BufferSize;
		rr = max(it.GetLastX(), it.GetCurX()) + BufferSize;
		rb = min(it.GetLastY(), it.GetCurY()) - BufferSize;
		rt = max(it.GetLastY(), it.GetCurY()) + BufferSize;

		int cl = min(x, rl); // circumscribed rectangle's left-value
		int cb = min(y, rb); // ... and the bottom-value
		int cr = max(x + width, rr); // ... right-value
		int ct = max(y + height, rt); // ... top-value

		if(width + (rr - rl) > cr - cl && height + (rt - rb) > ct - cb)
			return(true);
	} while(it.ToNext());

	return(false);
}

void BlockConnector::WriteXML(XMLWriter *xml)
{
	xml->OpenTag("BlockConnector");
	ChartItem::WriteXML(xml);
	xml->TextTag("StartX", startX);
	xml->TextTag("StartY", startY);
	xml->TextTag("EndX", endX);
	xml->TextTag("EndY", endY);
	if(endItem != NULL)
		xml->TextTag("EndItem", endItem->GetId());
	
	xml->OpenTag("MoveBys");
	for(auto it = parts.begin(); it != parts.end(); ++it) {
		xml->TextTag("MoveBy", it->GetMoveBy());
	}
	xml->CloseTag("MoveBys");
	
	xml->TextTag("ToRight", toRight);
	xml->CloseTag("BlockConnector");
}

void BlockConnector::ReadXML(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *idRefs)
{
	xml->OpenTag("BlockConnector");
	ChartItem::ReadXML(xml, subRefs, idRefs);
	xml->TextTag("StartX", &startX);
	xml->TextTag("StartY", &startY);
	xml->TextTag("EndX", &endX);
	xml->TextTag("EndY", &endY);
	int toRightTmp = 0;
	xml->TextTag("ToRight", &toRightTmp);
	toRight = toRightTmp != 0;
	
	if(xml->GetCurrentHasChild("EndItem")) {
		int endId;
		xml->TextTag("EndItem", &endId);
		// we write the id temporarly to the memory of the endItem pointer
		endItem = (ChartItem *) (long) endId;
	}
	SetEndPoint(endX, endY);

	if(xml->GetCurrentHasChild("MoveBys")) {
		xml->OpenTag("MoveBys");
		auto it = parts.begin();
		while(xml->GetCurrentHasChild("MoveBy")) {
			int moveByValue;
			xml->TextTag("MoveBy", &moveByValue);
			if(it != parts.end()) {
				it->SetMoveBy(moveByValue);
				++it;
			}
		}
		xml->CloseTag("MoveBys");
	}

	xml->CloseTag("BlockConnector");
}

void BlockConnector::ReadXML(IdItemMap *refs)
{
	if(endItem != NULL) {
		// the conversation is necessary on 64-bit systems
		IdItemMap::iterator it = refs->find((int) (long) endItem);

		if(it != refs->end()) {
			endItem = it->second;
			endItem->InputConnectorAdd(this);
		}
		// TODO: else throw exception
	}
}

// In most cases a connector does not generate basic-code
// But if the endItem is referenced several times we need to create a goto!
void BlockConnector::WriteCode(Compiler::Program &program)
{
	// if there is no end item, we do nothing here
	if(endItem) {
		// the endItem has been printed already if it has an label
		if(!endItem->GetLocationLabel().empty()) {
			// so just go to there
			program.LocalJump(Compiler::UnknownAddress(endItem->GetLocationLabel()));
		} else { // this item has not been referenced before...

			// are there any further references to this item? If so, just put
			// a label that it can be referenced again

			// we use the size of the input connectors-vector to know this.
			// Although that number might be too big if the other connectors 
			// that end here too come from items that are not connected with 
			// the start-item. But this is not a real problem because this
			// would just mean that there are some unused labels in the code...
			if(endItem->GetInputConnectorCount() > 1)
				endItem->SetLocationLabel(program.LabelCurrentLocation());

			// ...and generate it
			endItem->WriteCode(program);
		}
	} else {
		throw CodeWriteException(this, TR_CONNECTOR_NOT_CONNECTED_WITH_ANOTHER_BLOCK);
	}
}
