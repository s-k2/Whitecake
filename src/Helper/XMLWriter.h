#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <stdio.h> // i love it much more than c++'s iostream
#include <string>

class XMLWriter
{
public:
	explicit XMLWriter(const std::string &name);
	~XMLWriter();

	bool IsOk()
		{ return(f != NULL); };

	// use char * to reduce useless conversions (see XMLReader for details)
	void OpenTag(const char *name);
	void TextTag(const char *name, std::string value);
	void TextTag(const char *name, int value);
	void CloseTag(const char *name);

	inline void OpenTag(const std::string &name)
		{ OpenTag(name.c_str()); };
	inline void TextTag(const std::string &name, std::string value)
		{ TextTag(name.c_str(), value); };
	inline void TextTag(const std::string &name, int value)
		{ TextTag(name.c_str(), value); };
	inline void CloseTag(const std::string &name)
		{ CloseTag(name.c_str()); };

private:
	FILE *f;
	
	// this string always points to the space characters that are needed to indent
	const char *indentChars;
	static const char *Spaces; // a string full of spaces
	static const int SpacesCount;
};

#endif /* XMLWRITER_H */