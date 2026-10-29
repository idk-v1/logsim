#pragma once

#include "connector.h"
#include "logicblock.h"

#include <SDL3/SDL.h>

#include <vector>
#include <string>

struct LogicSim
{
	LogicSim();

	LogicBlock& getBoard();

	void recalc();

	void connect(LogicBlock& a, const std::string& aIndex, LogicBlock& b, 
		const std::string& bIndex, bool aIn = false, bool bIn = true);

	LogicBlock& add(const LogicBlock& block);

	void tick(int mousex, int mousey, bool click);

	void draw(SDL_Surface* surface);

private:
	LogicBlock board;
	std::vector<Connector> connections;
	std::vector<Connector> connectionsIn;
	std::vector<Connector> connectionsOut;
	std::vector<Connector> connectionsInOut;
};