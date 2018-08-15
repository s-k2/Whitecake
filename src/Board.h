#ifndef BOARD_H
#define BOARD_H

#include <map>
#include <string>
#include <vector>

typedef uint16_t Address;

class Board
{
public:
	Board();

	Address GetAddressOfIo(const std::string &ioRegister) const
	{
		auto it = ioRegisterToAddress.find(ioRegister);
		if(it == ioRegisterToAddress.end())
			throw std::runtime_error("Could not find port " + ioRegister);

		return(it->second);
	}
	inline const std::vector<uint8_t> &GetStdlib() { return(stdlib); }
	inline const std::map<std::string, Address> &GetStdlibSymbols() { return(stdlibSymbols); }

private:
	const std::map<std::string, Address> &ioRegisterToAddress;
	const std::vector<uint8_t> &stdlib;
	const std::map<std::string, Address> &stdlibSymbols;
};

#endif /* BOARD_H */
