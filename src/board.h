#pragma once

#include "connector.h"
#include "logicblock.h"

#include <SDL3/SDL.h>

#include <vector>
#include <string>

struct Board
{
	LogicBlock board;
	std::vector<Connector> connections;
	std::vector<Connector> connectionsIn;
	std::vector<Connector> connectionsOut;
	std::vector<Connector> connectionsInOut;

	Board();

	// a.out -> b.in
	void connect(LogicBlock& a, const std::string& aIndex, LogicBlock& b, 
		const std::string& bIndex, bool aIn = false, bool bIn = true);

	// a.in -> a.child.in
	//void connectToIn(LogicBlock& a, const std::string& aIndex, LogicBlock& b, const std::string& bIndex);

	// a.child.out -> a.out
	//void connectToOut(LogicBlock& a, const std::string& aIndex, LogicBlock& b, const std::string& bIndex);

	// a.in -> a.out
	//void connectInOut(LogicBlock& block, const std::string& indexIn, const std::string& indexOut);

	// same as board.board.add(block)
	LogicBlock& add(const LogicBlock& block);

	void tick();

	void draw(SDL_Surface* surface);
};