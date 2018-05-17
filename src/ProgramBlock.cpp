#include "ProgramBlock.h"

#include "BlockConnector.h"

ProgramBlock::ProgramBlock(Sub *sub, int x, int y, int width, int height)
	: ChartItem(sub, x, y, width, height)
{
	isDependent = false;
}

ProgramBlock::ProgramBlock(Sub *sub, int width, int height)
	: ChartItem(sub, width, height)
{
	isDependent = false;
}

bool ProgramBlock::WantsDrag(int x, int y)
{
	return(false);
}

bool ProgramBlock::HasConnectionPoint()
{
	// Hardly all program blocks have a connection point
	return(true);
}

