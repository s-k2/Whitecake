#include "ChartItem.h"

#include <utility>
using std::pair;

#include "Assignment.h"
#include "BlockConnector.h"
#include "IfBlock.h"
#include "Instruction.h"
#include "Sub.h"
#include "ProgramBlock.h"
#include "ProgramFlow.h"
#include "Helper/BasicWriter.h"
#include "Helper/XMLReader.h"
#include "Helper/XMLWriter.h"

ChartItem::ChartItem(Sub *sub, int x, int y, int width, int height)
{
	this->sub = sub;
	item.itemData = this;

	CanvasItemAdd(sub->GetCanvas(), &item);
	CanvasItemSetSize(GetSub()->GetCanvas(), &item, width, height);
	CanvasItemSetPos(GetSub()->GetCanvas(), &item, x, y);
}

ChartItem::ChartItem(Sub *sub, int width, int height)
{
	this->sub = sub;
	item.itemData = this;
	CanvasItemAdd(sub->GetCanvas(), &item);

	CanvasItemSetSize(GetSub()->GetCanvas(), &item, width, height);
}

ChartItem::~ChartItem()
{
	// delete all connectors that belong to this item
	if(dependentItems[0] != NULL) {
		delete dependentItems[0];

		if(dependentItems[1] != NULL)
			delete dependentItems[1];
	}

	// and release all connection's that ended up here
	for(size_t i = 0; i < inputConnectors.size(); i++) {
		inputConnectors[i]->ClearEndItem();
	}

	CanvasItemRemove(sub->GetCanvas(), &item);
}

void ChartItem::OnDrag(int x, int y)
{
	// Nothing to do here
}

bool ChartItem::HasConnectionPoint()
{
	return(false);
}

bool ChartItem::OnEdit(NativeWindow parent)
{
	return(false);
}

void ChartItem::Move(int relX, int relY)
{
	// if there is one (ore more) dependend item we have to move them too
	if(dependentItems[0] != NULL) {
		dependentItems[0]->Move(relX, relY);

		if(dependentItems[1] != NULL) {
			dependentItems[1]->Move(relX, relY);
		}
	}

	// move the item itself
	CanvasItemSetPosRelative(GetSub()->GetCanvas(), &item, relX, relY);

	// and if there are any connectors that end here, update the position
	for(size_t i = 0; i < inputConnectors.size(); i++) {
		inputConnectors[i]->UpdateMovedEndItem();
	}
}

void ChartItem::WriteXML(XMLWriter *xml)
{
	xml->OpenTag("ChartItem");

	// for cross-references
	xml->TextTag("Id", GetId());

	// width ore height aren't needed (they are fixed for each type of an item)
	xml->TextTag("X", CanvasItemGetX(&item));
	xml->TextTag("Y", CanvasItemGetY(&item));

	if(dependentItems[0] != NULL) {
		xml->OpenTag("DependendItems");
		dependentItems[0]->WriteXML(xml);

		if(dependentItems[1] != NULL) {
			dependentItems[1]->WriteXML(xml);
		}
		xml->CloseTag("DependendItems");
	}
	xml->CloseTag("ChartItem");
}

void ChartItem::ReadXML(XMLReader *xml, 
	SubReferenceVector *subRefs, IdItemMap *idRefs)
{
	xml->OpenTag("ChartItem");
	xml->TextTag("Id", &id);
	idRefs->insert(pair<int, ChartItem *>(id, this));

	int x, y;
	xml->TextTag("X", &x);
	xml->TextTag("Y", &y);
	CanvasItemSetPos(GetSub()->GetCanvas(), &item, x, y);

	if(xml->GetCurrentHasChild("DependendItems")) {
		xml->OpenTag("DependendItems");

		dependentItems[0] = GetSub()->CreateFromXMLChild(xml, 
			subRefs, idRefs);

		if(xml->GetCurrentNode()->GetChildCount() > 0)
			dependentItems[1] = GetSub()->CreateFromXMLChild(xml, 
			subRefs, idRefs);

		xml->CloseTag("DependendItems");
	}

	xml->CloseTag("ChartItem");
}

void ChartItem::ReadXML(IdItemMap *refs)
{
	// nothing to do here...
	// only some derived classes overwrite this method with real code
}

void ChartItem::WriteBasic(BasicWriter *basic)
{
	// continue with the dependend items
	if(dependentItems[0] != NULL) {
		dependentItems[0]->WriteBasic(basic);

		if(dependentItems[1] != NULL) {
			dependentItems[1]->WriteBasic(basic);
		}
	}
}

void ChartItem::WriteCode(Compiler::Program &program)
{
	// continue with the dependend items
	if(dependentItems[0] != NULL) {
		dependentItems[0]->WriteCode(program);

		if(dependentItems[1] != NULL) {
			dependentItems[1]->WriteCode(program);
		}
	}
}
