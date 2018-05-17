#ifndef XMLREADER_H
#define XMLREADER_H

#include <stdio.h> // i love it much more than c++'s iostream
#include <string>
#include <vector>

class XMLReaderException
{
public:
	explicit XMLReaderException(const char *description)
		{ this->description = description; };
	inline const char *GetDescription()
		{ return(description); };
private:
	const char *description;
};

class XMLNode {

private:
	std::string name;
	std::vector<XMLNode *> childs;
	std::string content;

public:
	~XMLNode();

	inline void AppendToContent(const char *str, int len)
		{ content.append(str, len); };
	inline std::string &GetContent()
		{ return(content); };
	inline void SetName(const char *name)
		{ this->name = name; };
	inline const std::string &GetName()
		{ return(name); };
	inline void AddChild(XMLNode *node)
		{ childs.push_back(node); };
	inline size_t GetChildCount()
		{ return(childs.size()); };
	inline XMLNode *GetChild(size_t index)
		{ return(childs[index]); };
	inline void RemoveChild(size_t index)
		{ childs.erase(childs.begin() + index); };
};

class XMLReader
{
public:
	explicit XMLReader(const std::string &path);
	~XMLReader();

	bool IsOk()
		{ return(ok); };

	inline XMLNode *GetCurrentNode()
		{ return(currentNode); };
	bool GetCurrentHasChild(const char *name);

	// we make an exception with strings here! We just use simple char-pointers
	// instead of string for the tag-names. This is to not create strings 
	// out of const literals that will be converted to char * again...
	void OpenTag(const char *name);
	void TextTag(const char *name, std::string *value);
	void TextTag(const char *name, int *value);
	void CloseTag(const char *name);

	inline void OpenTag(const std::string &name)
		{ OpenTag(name.c_str()); };
	inline void TextTag(const std::string &name, std::string *value)
		{ TextTag(name.c_str(), value); };
	inline void TextTag(const std::string &name, int *value)
		{ TextTag(name.c_str(), value); };
	inline void CloseTag(const std::string &name)
		{ CloseTag(name.c_str()); };

private:
	bool ok;
	// the root of the xml document (this node is not part of the xml file, its 
	// been inserted while parsing to have a root for everything)
	XMLNode *documentNode; 

	XMLNode *currentNode;
	std::vector<XMLNode *> nodesStack;

	static void StartHandler(void *data, const char *el, const char **attr);
	static void EndHandler(void *data, const char *el);
	static void CharHandler(void *userData, const char *str, int len);
};

#endif /* XMLREADER_H */