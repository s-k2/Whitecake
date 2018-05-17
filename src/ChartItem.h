#ifndef CHART_ITEM_H
#define CHART_ITEM_H

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Helper/Canvas.h"
#include "Platform.h"
#include "Sub.h"

class BasicWriter;
class BlockConnector;
class XMLReader;
class XMLWriter;
class Project;
class ChartItem;
namespace Compiler {
	class Program;
}

typedef std::vector<std::pair<Sub **, std::string> > SubReferenceVector;
typedef std::map<int, ChartItem *> IdItemMap;

class ChartItem
{
	// All things that have to do something with the graphical representation
protected:
	ChartItem(Sub *sub, int x, int y, int width, int height);
	ChartItem(Sub *sub, int width, int height); // for serialization
public:
	virtual ~ChartItem();

public:
	enum PaintFlags { PaintNotSelected = 1, PaintSelected = 2, PaintErrorMark = 4 };
	virtual void OnPaint(Drawing *drawing, int flags) = 0;
	virtual void Move(int relX, int relY);
	virtual bool WantsDrag(int x, int y) = 0;
	virtual void OnDrag(int x, int y);
	virtual bool IsInItem(int x, int y) = 0;
	virtual bool IsInItem(int x, int y, int width, int height) = 0;
	virtual bool HasConnectionPoint();
	virtual bool OnEdit(NativeWindow parent);

	inline bool IsDependent()
		{ return(isDependent); };
	inline bool HasDepedendent(ChartItem *item)
		{ return(item && (dependentItems[0] == item || dependentItems[1] == item)); };

	CanvasItem *GetCanvasItem()
		{ return(&item); }

	inline int GetWidth()
		{ return(CanvasItemGetWidth(&item)); };
	inline int GetHeight()
		{ return(CanvasItemGetHeight(&item)); };
	inline int GetX()
		{ return(CanvasItemGetX(&item)); };
	inline int GetY()
		{ return(CanvasItemGetY(&item)); };

	inline Sub *GetSub()
		{ return(sub); };
	inline Project *GetProject()
		{ return(GetSub()->GetProject()); };

	inline void InputConnectorAdd(BlockConnector *connector)
		{ inputConnectors.push_back(connector); };
	inline void InputConnectorRemove(BlockConnector *connector)
		{ inputConnectors.erase(find(inputConnectors.begin(), 
			  inputConnectors.end(), connector)); };
	inline size_t GetInputConnectorCount()
		{ return(inputConnectors.size()); };

	virtual void WriteXML(XMLWriter *xml);
	// Reading must be done in two steps... at first, read the contents of the
	// xml file. Then we must update the references to other ChartItems
	virtual void ReadXML(XMLReader *reader, 
		SubReferenceVector *subRefs, IdItemMap *idRefs);
	virtual void ReadXML(IdItemMap *refs);
	virtual void WriteBasic(BasicWriter *basic);
	virtual void WriteCode(Compiler::Program &program);

	// mark as deprecated
	void SetId(int id)
		{ this->id = id; };
	int GetId() 
		{ return(id); };

	void SetLocationLabel(const std::string &locationLabel)
		{ this->locationLabel = locationLabel; };
	const std::string &GetLocationLabel()
		{ return(locationLabel); };
protected:
	Sub *sub;
	CanvasItem item;
	

	// The dependent items will be removed when this item is removed and moved
	// when this item moves... this are always the connectors of some blocks
	// ... yes, it's bad to use this fixed maximum number, but we don't need more
	ChartItem *dependentItems[2];

	// Is it possiple that the user deletes/moves/... this item directly?
	bool isDependent;

	// connectors that end up in this item
	std::vector<BlockConnector *> inputConnectors;
	
	int id; // this is used temporary for saving/loading and code generation
	std::string locationLabel;
};

#endif /* CHART_ITEM_H */
