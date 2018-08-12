#ifndef CODE_WRITE_EXCEPTION_H
#define CODE_WRITE_EXCEPTION_H

#include <string>

class ChartItem;
// exceptions that happen while writing basic-code
class CodeWriteException
{
public:
	CodeWriteException(ChartItem *item, const std::string &text)
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

#endif /* CODE_WRITE_EXCEPTION_H */
