#ifndef PROGRAM_BLOCK_H
#define PROGRAM_BLOCK_H

#include "ChartItem.h"

class ProgramBlock : public ChartItem
{
public:
	virtual bool WantsDrag(int x, int y);
	virtual bool HasConnectionPoint();

protected:
	ProgramBlock(Sub *sub, int x, int y, int width, int height);
	ProgramBlock(Sub *sub, int width, int height); // for serialization
};

#endif /* PROGRAM_BLOCK_H */