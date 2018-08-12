#ifndef BLOCKCONNECTOR_H	
#define BLOCKCONNECTOR_H

#include "ChartItem.h"

#include <vector>

#include "Helper/Geometry.h"

class ConnectorPart
{
public:
	ConnectorPart(Direction direction, int moveBy = 0)
		: direction(direction), moveBy(moveBy)
	{
	}

	void Draw(Path &path)
	{
	}

	const Direction &GetDirection() const
		{ return(direction); };

	int GetMoveBy() const
		{ return(moveBy); };
	void SetMoveBy(int newMoveBy)
		{ moveBy = newMoveBy; };

private:
	Direction direction;
	int moveBy;
};

class BlockConnector : 	public ChartItem
{
public:
	enum Orientation { DownRight, Down, DownLeft, 
		               TopLeft, Top, TopRight };
	// TODO: We just support down and right orientation
	BlockConnector(Sub *sub, int x, int y, bool toRight = 0);
	explicit BlockConnector(Sub *sub); // don't create dependend items
	virtual ~BlockConnector(void);
	
	virtual void Move(int relX, int relY);
	void UpdateMovedEndItem();
	inline void ClearEndItem() // is used if the endPoint item is deleted
		{ endItem = NULL; };
	inline ChartItem *GetEndItem()
		{ return(endItem); };
	inline int GetEndX()
		{ return(endX); };
	inline int GetEndY()
		{ return(endY); };

	virtual bool WantsDrag(int x, int y);
	virtual void OnDrag(int x, int y);

	void ConnectTo(ChartItem *item);

	virtual void OnPaint(Drawing *drawing, int flags);
	virtual bool IsInItem(int x, int y);
	virtual bool IsInItem(int x, int y, int width, int height);
	
	void WriteXML(XMLWriter *xml);
	void ReadXML(XMLReader *reader, SubReferenceVector *subRefs, IdItemMap *idRefs);
	void ReadXML(IdItemMap *refs);
	void WriteCode(Compiler::Program &program);

private:
	static const int Width, Height;

	bool toRight;
	ChartItem *endItem;
	int endX, endY; // absolute
	int startX, startY; // absolute
	bool draggingEndPoint;
	ConnectorPart *movedPart;
	int movePartInitialX, movePartInitialY;

	void AdjustEndToPlugable(int absX, int absY);
	void SetEndPoint(int absX, int absY);

	static void FindPlugableItem(Canvas *canvas, CanvasItem *item, void *userData, int x, int y);

	void DetermineParts();
	void UpdateCanvasItem();
	std::vector<ConnectorPart> parts;
	Direction partsDirection;

	ConnectorPart *GetPartAt(int x, int y);
};

#endif /* BLOCKCONNECTOR_H */
