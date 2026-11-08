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

	void tick(int mousex, int mousey, int oldmousex, int oldmousey, bool click, bool oldclick);

	void draw(SDL_Surface* surface);

	void setZoom(int scale, int screenw, int screenh);
	void zoom(int scale, int screenw, int screenh);
	void setScroll(int x, int y);
	void scroll(int x, int y);

private:
	bool needsRecalc = true;
	int scale = 0;
	int fontw = 0;
	int fonth = 0;
	int scrollx = 0;
	int scrolly = 0;

	LogicBlock* held = NULL;

	LogicBlock board;
	std::vector<Connector> connections;
	std::vector<Connector> connectionsIn;
	std::vector<Connector> connectionsOut;
	std::vector<Connector> connectionsInOut;
};