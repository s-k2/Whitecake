#ifndef PROJECT_H
#define PROJECT_H

#include <vector>
#include <string>

class Sub;
namespace Compiler {
	class Program;
}

#include "Variables.h"

class Project
{
public:
	Project();
	explicit Project(std::string path, bool *valid = NULL);
	~Project();

	inline size_t GetChartsCount()
		{ return(charts.size()); };
	inline Sub *GetChart(size_t index)
		{ return(charts[index]); };
	Sub *AddChart();
	void DeleteChart(Sub *sub);
	inline Sub *GetStartSub()
		{ return(charts[0]); };

	bool WriteXML(const std::string &path);
	bool ReadXML(const std::string &path);
	void WriteCode(Compiler::Program &program);

	inline const Variables &GetVariables() const
		{ return(variables); };
	inline Variables *GetVariablesPtr() // TODO: This should look different!
		{ return(&variables); }

	inline const std::string &GetPath()
		{ return(path); };


private:
	std::vector<Sub *> charts;
	
	// this objects holds all information about the available variables
	Variables variables;

	std::string path;
};

#endif /* PROJECT_H */
