#include "XMLReader.h"

#include <string.h>
#include <expat.h>
#include <string>
using std::string;

#include "../Translate.h"

#include <windows.h>

XMLNode::~XMLNode()
{
	for(size_t i = 0; i < childs.size(); i++)
		delete childs[i];
}

void XMLReader::StartHandler(void *data, const char *tag, const char **attr)
{
	register XMLReader *tree = (XMLReader *) data;

	XMLNode *newNode = new XMLNode;
	newNode->SetName(tag);

	tree->currentNode->AddChild(newNode);
	tree->nodesStack.push_back(tree->currentNode);
	tree->currentNode = newNode;
}

void XMLReader::EndHandler(void *data, const char *tag)
{
	register XMLReader *tree = (XMLReader *) data;

	tree->currentNode = tree->nodesStack.back();
	tree->nodesStack.pop_back();
}

void XMLReader::CharHandler(void *data, const char *str, int len)
{
	register XMLReader *tree = (XMLReader *) data;

	tree->currentNode->AppendToContent(str, len);
}

XMLReader::XMLReader(const string &path)
{
	XML_Parser p = XML_ParserCreate(NULL);
	if(p == NULL)
		throw XMLReaderException(TR_XML_PARSERCREATE_ERROR);

	XML_SetUserData(p, this);
	XML_SetElementHandler(p, StartHandler, EndHandler);
	XML_SetCharacterDataHandler(p, CharHandler);
	
	char buffer[4096];
	FILE *f;
	if((f = fopen(path.c_str(), "r")) == NULL) {
		XML_ParserFree(p);
		throw XMLReaderException(TR_IO_ERROR);
	}

	bool done = false;
	size_t len;

	documentNode = currentNode = new XMLNode;

	while(!done) {
		len = (size_t) fread(buffer, 1, sizeof(buffer), f);
		done = len < sizeof(buffer);

		if(!XML_Parse(p, buffer, len, done)) {
			XML_ParserFree(p);
			throw XMLReaderException(TR_PARSE_ERROR);
		}
	}
	XML_ParserFree(p);

	currentNode = documentNode;
	nodesStack.push_back(documentNode);

	fclose(f);
}

XMLReader::~XMLReader()
{
	currentNode = 0;
	delete documentNode;
}

bool XMLReader::GetCurrentHasChild(const char *name)
{
	for(size_t i = 0; i < currentNode->GetChildCount(); i++) {
		if(currentNode->GetChild(i)->GetName() == name) {
			return(true);
		}
	}
	return(false);
}

void XMLReader::OpenTag(const char *name)
{
	for(size_t i = 0; i < currentNode->GetChildCount(); i++) {
		if(currentNode->GetChild(i)->GetName() == name) {
			nodesStack.push_back(currentNode);
			currentNode = currentNode->GetChild(i);
			return;
		}
	}
	throw XMLReaderException(TR_CANNOT_FIND_TAG_TO_OPEN_TAG);
}

void XMLReader::TextTag(const char *name, string *value)
{
	for(size_t i = 0; i < currentNode->GetChildCount(); i++) {
		register XMLNode *currentChild = currentNode->GetChild(i);
		
		if(currentChild->GetName() == name) {
			std::string utf8_str = currentChild->GetContent();
			int size = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(),
					utf8_str.length(), nullptr, 0);
			std::wstring utf16_str(size, '\0');
			MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(),
					utf8_str.length(), &utf16_str[0], size);


			int ansi_size = WideCharToMultiByte(CP_ACP, 0, utf16_str.c_str(),
					utf16_str.length(), nullptr, 0,
					nullptr, nullptr);
			std::string ansi_str(ansi_size, '\0');
			WideCharToMultiByte(CP_ACP, 0, utf16_str.c_str(),
					utf16_str.length(), &ansi_str[0], ansi_size,
					nullptr, nullptr);

			*value = ansi_str;

			// If this TextTag has childs something's wrong... We just accept
			// a very strict XML so throw an error in such a case
			if(currentChild->GetChildCount() > 0)
				throw XMLReaderException(TR_TEXT_TAG_WITH_CHILDS);

			// delete this node, now it should be no problem
			currentNode->RemoveChild(i);
			delete currentChild;

			return;
		}
	}
	throw XMLReaderException(TR_CANNOT_FIND_TAG_FOR_TEXT_TAG);
}

void XMLReader::TextTag(const char *name, int *value)
{
	for(size_t i = 0; i < currentNode->GetChildCount(); i++) {
		register XMLNode *currentChild = currentNode->GetChild(i);

		if(currentChild->GetName() == name) {
			char *success;
			*value = (int) strtol(
				currentChild->GetContent().c_str(), &success, 10);
			if(success == NULL || strlen(success) != 0)
				throw XMLReaderException(TR_INVALID_NUMBER_IN_TEXT_TAG);

			// If this TextTag has childs something's wrong... We just accept
			// a very strict XML so throw an error in such a case
			if(currentChild->GetChildCount() > 0)
				throw XMLReaderException(TR_TEXT_TAG_WITH_CHILDS);

			// delete this node, now it should be no problem
			currentNode->RemoveChild(i);
			delete currentChild;

			return;
		}
	}
	throw XMLReaderException(TR_CANNOT_FIND_TAG_FOR_TEXT_TAG);
}

void XMLReader::CloseTag(const char *name)
{
	XMLNode *oldNode = currentNode;
	currentNode = nodesStack.back();
	nodesStack.pop_back();
	
	// normally the first tag is what we're looking for so this loop isn't as 
	// time-consuming as it seems
	for(size_t i = 0; i < currentNode->GetChildCount(); i++) {
		register XMLNode *currentChild = currentNode->GetChild(i);
		if(currentChild == oldNode) {

			// Although the user wants to close the tag there are items in it
			// that have not been read already... Throw an error because if the
			// these remaining xml-nodes are invalid and cannot be read
			if(currentChild->GetChildCount() > 0)
				throw XMLReaderException(TR_CLOSING_TAG_WITH_CHILDS);

			currentNode->RemoveChild(i);
			delete oldNode;
			return;
		}
	}

	throw XMLReaderException(
		TR_WELL_I_WANTED_TO_REMOVE_A_NODE_THAT_ALREADY_HAS_BEEN_DELETED);
}
