#include "Project.h"

#include <string>
using std::string;
#include <vector>
using std::vector;

#include "ChartItem.h"
#include "Sub.h"
#include "Compiler.h"
#include "Helper/XMLReader.h"
#include "Helper/XMLWriter.h"
#include "Settings.h"

Project::Project()
{
}

Project::Project(string path, bool *valid)
{
	this->path = path;
	if(valid)
		*valid = ReadXML(path);
}

Project::~Project()
{
	while(charts.size() > 0) {
		delete charts.back();
		charts.erase(charts.end() - 1);
	}
}

Sub *Project::AddChart()
{
	Sub *chart = new Sub(this);
	charts.push_back(chart);

	return(chart);
}

void Project::DeleteChart(Sub *sub)
{
	for(vector<Sub *>::iterator it = charts.begin(); it != charts.end(); it++) {
		if(sub == *it) {
			charts.erase(it);
			delete sub;
			return;
		}
	}
}

bool Project::WriteXML(const string &path)
{
	XMLWriter xml(path);
	if(!xml.IsOk())
		return(false);

	xml.OpenTag("Project");

	variables.Write(&xml);
	
	xml.OpenTag("Subs");
	for(vector<Sub *>::iterator it = charts.begin(); it != charts.end(); it++)
		(*it)->WriteXML(&xml);
	xml.CloseTag("Subs");

	xml.CloseTag("Project");
	
	this->path = path;

	return(true);
}

bool Project::ReadXML(const string &path)
{
	try {
		XMLReader xml(path);

		SubReferenceVector subRefs;

		xml.OpenTag("Project");

		variables.Read(&xml);

		xml.OpenTag("Subs");
		while(xml.GetCurrentNode()->GetChildCount() > 0) {
			Sub *chart = new Sub(this, &xml, &subRefs);
			charts.push_back(chart);
		}
		xml.CloseTag("Subs");

		xml.CloseTag("Project");

		for(SubReferenceVector::iterator it = subRefs.begin(); it != subRefs.end(); it++) {
			for(vector<Sub *>::iterator chartsIt = charts.begin(); chartsIt != charts.end(); chartsIt++) {
				if(it->second == (*chartsIt)->GetName()) {
					*(it->first) = *chartsIt;

					// there is no need to iterate any longer through all available subs... 
					// so we continue with the next subReference
					continue; 
				}
			}			
		}
	} catch(XMLReaderException&) {
		return(false);
	}

	return(true);
}

void Project::WriteCode(Compiler::Program &program)
{
	// be aware that we must start with the Main-function first!
	for(vector<Sub *>::iterator it = charts.begin(); it != charts.end(); it++)
		(*it)->WriteCode(program);
}
