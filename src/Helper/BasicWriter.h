#ifndef BASIC_WRITER_H
#define BASIC_WRITER_H

#include <stdio.h> // i love it much more than c++'s iostream
#include <string>

class ChartItem;
// exceptions that happen while writing basic-code
class WriteBasicException
{
public:
	WriteBasicException(ChartItem *item, const std::string &text)
	{
		this->item = item;
		this->text = text;
	};

	inline ChartItem *GetItem()
		{ return(item); };
	inline const std::string &GetText()
		{ return(text); };
private:
	std::string text;
	ChartItem *item;
};

class BasicWriter
{
public:
	explicit BasicWriter(const std::string &name);
	~BasicWriter();

	bool IsOk()
		{ return(f != NULL); };

	inline void PutLabel(int id)
		{ fprintf(f, "A%.8X:\n", id); };
	inline void GotoLabel(int id)
		{ fprintf(f, "Goto A%.8X\n", id); };
	inline int CreateLabelId()
		{ return(++nextId); };
	
	inline void PutCode(const char *code) // for literals
		{ fprintf(f, "%s\n", code); };
	inline void PutCode(const std::string &code)
		{ PutCode(code.c_str()); };
	void PutCodeVarArgs(const char *code, ...);
	inline void CallSub(const std::string &sub)
		{ fprintf(f, "Call %s\n", sub.c_str()); };
	inline void PutIf(const std::string &condition, int yesId, int noId)
		{ fprintf(f, "If %s Then\n"
		      "  Goto A%.8X\n"
			  "Else\n"
			  "  Goto A%.8X\n"
			  "End If\n", condition.c_str(), yesId, noId); };
	inline void DeclareSub(const std::string &name)
		{ fprintf(f, "Declare Sub %s()\n", name.c_str()); };
	inline void StartSub(const std::string &name)
		{ fprintf(f, "Sub %s()\n", name.c_str()); };
	inline void EndSub()
		{ fprintf(f, "End Sub\n\n"); };


private:
	FILE *f;
	int nextId; // used to create new label-ids
};

#endif /* BASIC_WRITER_H */