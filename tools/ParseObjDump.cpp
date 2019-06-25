#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>
#include <map>

using namespace std;

map<string, size_t> allSymbols;

void ParseTokens(vector<string> &tokens)
{
	// if the last column looks like: ".hidden some_symbol", remove the hidden
	if(*(tokens.end() - 2) == ".hidden")
		tokens.erase(tokens.end() - 2);

	string section = *(tokens.end() - 3);
	string type = *(tokens.end() - 4);
	if(section == ".text") {
		allSymbols.insert(make_pair(tokens.back(), stoi(tokens.front(), NULL, 16)));
	} else if(section == ".bss") {
		allSymbols.insert(make_pair(tokens.back(), stoi(tokens.front(), NULL, 16) - 0x800000));
	} else if(section == "*ABS*" && *(tokens.begin() + 1) == "g") {
		allSymbols.insert(make_pair(tokens.back(), stoi(tokens.front(), NULL, 16)));
	}
}

void ParseLine(string line)
{
	std::stringstream ss(line);
	std::istream_iterator<std::string> begin(ss);
	std::istream_iterator<std::string> end;
	std::vector<std::string> tokens(begin, end);

	if(tokens.size() >= 5)
		ParseTokens(tokens);
	//std::copy(vstrings.begin(), vstrings.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
}

void ReadSymbolsFile(string filename)
{
	ifstream input(filename);
	string line;
	while(getline(input, line)) {
		ParseLine(line);
	}

	cout << "static const std::map<std::string, Address> StdlibSymbols = {" << endl;
	bool isFirst = true;
	for(auto &symbol : allSymbols) {
		if(!isFirst)
			cout << "," << endl;
		isFirst = false;

		cout << "\t{ \"" << symbol.first << "\", 0x" << std::hex << symbol.second << " }";
	}
	cout << endl << "};";
}

template<size_t Count>
int ParseHexNumber(string::iterator &it)
{
	int val = stoi(string(it, it + Count), NULL, 16);
	it += Count;

	return(val);
}

void ParseHexLine(string line)
{
	string::iterator it = line.begin();
	if(*it != ':')
		throw runtime_error("Invalid begin of hex-file-line");
	it++;

	size_t len = ParseHexNumber<2>(it);
	int address = ParseHexNumber<4>(it);
	int type = ParseHexNumber<2>(it);

	if(type != 0)
		return;

	for(size_t i = 0; i < len; i++) {
		int currentByte = ParseHexNumber<2>(it);
		if((address + i) > 0)
			cout << ", ";
		if(i == 0)
			cout << endl << '\t';

		cout << "0x" << setfill('0') << setw(2) << hex << currentByte;
	}
}

void ReadHexFile(string filename)
{
	ifstream input(filename);
	string line;

	cout << "static const std::vector<uint8_t> Stdlib = {";
	while(getline(input, line)) {
		ParseHexLine(line);
	}
	cout << "\n};\n";
	cout << endl;
}

int main(int argc, char *argv[])
{
	if(argc < 2) {
		cout << "usage: ParseObjDump [Basename]" << endl;
		return(1);
	}

	ReadHexFile(string(argv[1]) + ".hex");
	ReadSymbolsFile(string(argv[1]) + ".symbols");

	cout << endl;

	return(0);
}
