#include "board.h"

Board::Board()
{
	board.label = "Board";
}

void Board::connect(LogicBlock& a, const std::string& aIndex, LogicBlock& b, 
	const std::string& bIndex, bool aIn, bool bIn)
{
	connections.push_back(Connector(a, aIndex, b, bIndex, aIn, bIn));
}

LogicBlock& Board::add(const LogicBlock& block)
{
	return board.add(block);
}

void Board::tick()
{
	board.resetWrittenTo();

	for (auto& c : connections)
		c.tick();

	for (auto& c : connectionsIn)
		c.tick();

	board.tick();

	for (auto& c : connectionsInOut)
		c.tick();

	for (auto& c : connectionsOut)
		c.tick();
}

void Board::draw(SDL_Surface* surface)
{
	board.draw(surface);

	for (auto& c : connections)
		c.draw(surface);

	for (auto& c : connectionsIn)
		c.draw(surface);

	for (auto& c : connectionsOut)
		c.draw(surface);
}