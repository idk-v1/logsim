#include "logicsim.h"
#include "../graphics/bulletinV1_font.h"

LogicSim::LogicSim() : board("Board")
{ 
	if (scale)
	{
		fontw = font_w * scale;
		fonth = font_h * scale;
	}
	else
	{
		fontw = font_w / 2;
		fonth = font_h / 2;
	}
	LogicBlock::setFontSize(fontw, fonth, scale, (scale ? 2 * scale : 1));
}

LogicBlock& LogicSim::getBoard()
{
	return board;
}

void LogicSim::recalc()
{
	board.recalc(scrollx, scrolly, true);
	needsRecalc = false;
}

void LogicSim::connect(LogicBlock& a, const std::string& aIndex, LogicBlock& b,
	const std::string& bIndex, bool aIn, bool bIn)
{
	if (!aIn && bIn)
		connections.push_back(Connector(a, aIndex, b, bIndex, outin));
	else if (aIn && bIn)
		connectionsIn.push_back(Connector(a, aIndex, b, bIndex, inin));
	else if (!aIn && !bIn)
		connectionsOut.push_back(Connector(a, aIndex, b, bIndex, outout));
	else
		connectionsInOut.push_back(Connector(a, aIndex, b, bIndex, inout));
}

LogicBlock& LogicSim::add(const LogicBlock& block)
{
	return board.add(block);
}

void LogicSim::tick(int mousex, int mousey, int oldmousex, int oldmousey, bool click, bool oldclick)
{
	board.resetWrittenTo();

	for (auto& c : connections)
		c.tick();

	for (auto& c : connectionsIn)
		c.tick();

	bool found = false;
	board.tick(mousex, mousey, oldmousex, oldmousey, click, oldclick, found, &held);
	if (found || needsRecalc)
		recalc();
	else
		held = NULL;

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

void LogicSim::setZoom(int scale, int screenw, int screenh)
{
	if (scale >= 0)
	{
		// find coords on board in the center of the screen
		int centerx = screenw / 2 - scrollx;
		int centery = screenh / 2 - scrolly;
		float scaleDiff = (scale ? 2 * scale : 1) / (float)(this->scale ? 2 * this->scale : 1);
		scrollx -= centerx;
		scrolly -= centery;
		int oldx = scrollx;
		int oldy = scrolly;
		scrollx *= scaleDiff;
		scrolly *= scaleDiff;
		scrollx += oldx - scrollx + centerx / scaleDiff;
		scrolly += oldy - scrolly + centery / scaleDiff;

		printf("%f\n", scaleDiff);
		this->scale = scale;
		if (scale)
		{
			fontw = font_w * scale;
			fonth = font_h * scale;
		}
		else
		{
			fontw = font_w / 2;
			fonth = font_h / 2;
		}
		LogicBlock::setFontSize(fontw, fonth, scale, (scale ? 2 * scale : 1));
		recalc();
	}
}

void LogicSim::zoom(int scale, int screenw, int screenh)
{
	setZoom(this->scale + scale, screenw, screenh);
}

void LogicSim::setScroll(int x, int y)
{
	scrollx = x;
	scrolly = y;
}

void LogicSim::scroll(int x, int y)
{
	setScroll(scrollx + x * (scale ? 2 * scale : 1), scrolly + y * (scale ? 2 * scale : 1));
}
