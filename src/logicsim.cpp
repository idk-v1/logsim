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

void LogicSim::interact(int mousex, int mousey, int oldmousex, int oldmousey, bool click, bool oldclick)
{
	bool found = false;
	board.interact(mousex, mousey, oldmousex, oldmousey, click, oldclick, found, &held);
	if (found || needsRecalc)
		recalc();
	else
		held = NULL;
}

void LogicSim::draw(SDL_Surface* surface)
{
	board.draw(surface, held);

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
		float centerx = screenw / 2 - scrollx;
		float centery = screenh / 2 - scrolly;
		float scaleDiff = (scale ? scale : 0.5f) / (float)(this->scale ? this->scale : 0.5f);
		float cxf = centerx / (float)board.getSize().x;
		float cyf = centery / (float)board.getSize().y;
		scrollx -= centerx;
		scrolly -= centery;
		scrollx *= scaleDiff;
		scrolly *= scaleDiff;

		// f(2)   = -0.50
		// f(1)   = +0.00
		// f(0.5) = +0.25
		// slope = -0.5x + 0.5
		float screenmul = -0.5 * scaleDiff + 0.5;
		scrollx += cxf * board.getSize().x * scaleDiff + screenw * screenmul;
		scrolly += cyf * board.getSize().y * scaleDiff + screenh * screenmul;

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
