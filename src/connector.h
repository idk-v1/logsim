#pragma once

#include "logicblock.h"

#include <SDL3/SDL.h>

#include <map>
#include <string>

struct Connector
{
	Connector(LogicBlock& a, const std::string& aIndex, LogicBlock& b, const std::string& bIndex, bool aIn, bool bIn);

	void tick();

	void draw(SDL_Surface* surface);

	LogicBlock& a, & b;
	std::string aIndex, bIndex;
	bool aIn, bIn;
};

#define inout true, false
#define inin true, true
#define outout false, false
#define outin false, true