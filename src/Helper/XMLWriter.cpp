#include "XMLWriter.h"

#include <string>
using std::string;
#include "String.h"

#include <windows.h>

const char *XMLWriter::Spaces = "                                ";
const int XMLWriter::SpacesCount = 32;

XMLWriter::XMLWriter(const string &path)
{
	// we start with zero indent, this means an empty string
	indentChars = Spaces + SpacesCount; 

	f = fopen(path.c_str(), "w");
	if(f != NULL)
		fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
}

XMLWriter::~XMLWriter()
{
	fclose(f);
}

void XMLWriter::OpenTag(const char *name)
{
	fprintf(f, "%s<%s>\n", indentChars, name);

	if(indentChars != Spaces)
		indentChars--;
}

void XMLWriter::TextTag(const char *name, string value)
{
	std::string codepage_str = value;
	int size = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, codepage_str.c_str(),
								   codepage_str.length(), nullptr, 0);
	std::wstring utf16_str(size, '\0');

	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, codepage_str.c_str(),
						codepage_str.length(), &utf16_str[0], size);

	int utf8_size = WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(),
										utf16_str.length(), nullptr, 0,
										nullptr, nullptr);
	std::string utf8_str(utf8_size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(),
						utf16_str.length(), &utf8_str[0], utf8_size,
						nullptr, nullptr);

	String::Replace(&utf8_str, "&", "&amp;");
	String::Replace(&utf8_str, "<", "&lt;");
	fprintf(f, "%s<%s>%s</%s>\n", indentChars, name, utf8_str.c_str(), name);

/*	String::Replace(&value, "&", "&amp;");
	String::Replace(&value, "<", "&lt;");
	fprintf(f, "%s<%s>%s</%s>\n", indentChars, name, value.c_str(), name);*/
}

void XMLWriter::TextTag(const char *name, int value)
{
	fprintf(f, "%s<%s>%d</%s>\n", indentChars, name, value, name);
}

void XMLWriter::CloseTag(const char *name)
{
	if(indentChars != Spaces + SpacesCount)
		indentChars++;

	fprintf(f, "%s</%s>\n", indentChars, name);
}
