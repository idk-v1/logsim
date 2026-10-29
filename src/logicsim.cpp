#include "logicsim.h"

LogicSim::LogicSim() : board("Board") {}

LogicBlock& LogicSim::getBoard()
{
	return board;
}

void LogicSim::recalc()
{
	board.recalc(0, 0);
}

void LogicSim::connect(LogicBlock& a, const std::string& aIndex, LogicBlock& b,
	const std::string& bIndex, bool aIn, bool bIn)
{
	connections.push_back(Connector(a, aIndex, b, bIndex, aIn, bIn));
}

LogicBlock& LogicSim::add(const LogicBlock& block)
{
	return board.add(block);
}

void LogicSim::tick()
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

void LogicSim::draw(SDL_Surface* surface)
{
	board.draw(surface);

	for (auto& c : connections)
		c.draw(surface);

	for (auto& c : connectionsIn)
		c.draw(surface);

	for (auto& c : connectionsOut)
		c.draw(surface);
}