#ifndef SUB_H
#define SUB_H

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Helper/Canvas.h"
#include "Platform.h"
#include "Project.h"

class BasicWriter;
class Sub;
class ChartItem;
class ChartView;
class Microcontroller;
class StartBlock;
class XMLReader;
class XMLWriter;

namespace Compiler {
	class Program;
}

typedef std::vector<std::pair<Sub **, std::string> > SubReferenceVector;
typedef std::map<int, ChartItem *> IdItemMap;

class Sub
{

public:
	explicit Sub(Project *project);
	Sub(Project *project, XMLReader *reader, 
		SubReferenceVector *subRefs);
	~Sub();

	inline Canvas *GetCanvas()
		{ return(&canvas); };

	inline void SetStartBlock(StartBlock *block)
		{ startBlock = block; };
	inline StartBlock *GetStartBlock()
		{ return(startBlock); };

	void WriteXML(XMLWriter *xml);
	void ReadXML(XMLReader *reader, SubReferenceVector *subRefs);
	ChartItem *CreateFromXMLChild(XMLReader *xml, 
		SubReferenceVector *subRefs, IdItemMap *refs);
	void WriteBasic(BasicWriter *basic);
	void WriteCode(Compiler::Program &program);


	inline bool IsSelected(ChartItem *item)
		{ return(find(selectedItems.begin(), 
				     selectedItems.end(), item) != selectedItems.end()); };
	inline bool HasSelection()
		{ return(GetSelectionCount() > 0); }
	inline void ClearSelection()
		{ selectedItems.clear(); };
	inline void SelectItem(ChartItem *item)
		{ selectedItems.push_back(item); };
	inline size_t GetSelectionCount()
		{ return(selectedItems.size()); };
	inline ChartItem *GetSelectedItem(int index)
	    { return(selectedItems[index]); };

	inline ChartItem *GetErrorItem() const
		{ return(errorItem); };
	inline void SetErrorItem(ChartItem *item)
		{ errorItem = item; };

	inline const std::string &GetName()
		{ return(name); };
	inline void SetName(const std::string &newName)
		{ name = newName; };

	inline Project *GetProject()
		{ return(project); };

	static const char *FunctionPrefix;

private:
	Canvas canvas;

	// to what project does this object belong to?
	Project *project;

	// this is the block, where the code starts
	StartBlock *startBlock;

	// More than one item can be selected so we manage it as a stl-vector that
	// contains pointers to all items that are selected
	std::vector<ChartItem *> selectedItems;

	// this points to an object that caused an error during last compilation
	ChartItem *errorItem;

	// Helper functions used as callbacks for the IO-methods
	static void IdentifyItems(Canvas *canvas, 
		CanvasItem *item, void *userData);
	static void SaveItem(Canvas *canvas, CanvasItem *item, void *userData);
	static void ClearItemIdAndCheckUnconnected(Canvas *canvas, CanvasItem *item, void *userData);
	static void ClearItemLabelAndCheckUnconnected(Canvas *canvas, CanvasItem *item, void *userData);
	static void UpdateItemRefs(Canvas *canvas, 
		CanvasItem *item, void *userData);
	static void DeleteItem(Canvas *canvas, CanvasItem *item, void *userData);
	static void DeleteReferences(Canvas *canvas, CanvasItem *item, void *userData);

	// the name of this chart (e. g. subroutine-name)
	std::string name;
};

class EditSubDlg : public NativeDialog
{
public:
	EditSubDlg(NativeWindow parent, Sub *sub, ChartView *view);
	~EditSubDlg();

	virtual void PutControls();
	virtual bool OnOK();

	void OnDelete(NativeControl *sender);

private:
	Sub *sub;
	ChartView *view;

	NativeLabel *label;
	NativeEdit *edit;
	NativeButton *okButton;
	NativeButton *cancelButton;
	NativeButton *deleteButton;
};

#endif /* SUB_H */
