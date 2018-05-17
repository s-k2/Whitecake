#include "BasicWriter.h"

#include <stdarg.h>
#include <string>
using std::string;

BasicWriter::BasicWriter(const string &path)
{
	f = fopen(path.c_str(), "w");
	if(f != NULL)
		fprintf(f, "' auto-generated basic code...\n");
	
	nextId = 0;
}

BasicWriter::~BasicWriter()
{
	fclose(f);
}

void BasicWriter::PutCodeVarArgs(const char *code, ...)
{
	va_list args;
	va_start(args, code);

	vfprintf(f, code, args);
	fputc('\n', f);
	
	va_end(args);
}