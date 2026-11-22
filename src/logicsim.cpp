#include "logicsim.h"
#include "../graphics/bulletinV1_font.h"

LogicSim::LogicSim()
{ 
	std::vector<LogicBlock> temp;
	temp.reserve(12);
	temp.push_back(LogicBlockAnd);
	temp.push_back(LogicBlockOr);
	temp.push_back(LogicBlockNor);
	temp.push_back(LogicBlockNand);
	temp.push_back(LogicBlockXnor);
	temp.push_back(LogicBlockXor);
	temp.push_back(LogicBlockNot);
	temp.push_back(LogicBlockDelay);
	temp.push_back(LogicBlockOn);
	temp.push_back(LogicBlockOff);
	temp.push_back(LogicBlockToggle);
	temp.push_back(LogicBlockButton);

	LogicBlock::blockCount = 0;
	board = LogicBlock("", "", "Board");
	board.setExpand(true);

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
	if (isArray(aIndex) && getArrayNumber(aIndex) == -1 &&
		isArray(bIndex) && getArrayNumber(bIndex) == -1)
	{
		for (int i = 0; ; ++i)
		{
			std::string newAIndex = arrayToStr(arrayToName(aIndex), i);
			std::string newBIndex = arrayToStr(arrayToName(bIndex), i);
			if (a.getConnectionIndex(newAIndex, aIn) == -1 ||
				b.getConnectionIndex(newBIndex, bIn) == -1)
				break;
	
			if (!aIn && bIn)
				connections.push_back(Connector(&a, newAIndex, &b, newBIndex, outin));
			else if (aIn && bIn)
				connectionsIn.push_back(Connector(&a, newAIndex, &b, newBIndex, inin));
			else if (!aIn && !bIn)
				connectionsOut.push_back(Connector(&a, newAIndex, &b, newBIndex, outout));
			else
				connectionsInOut.push_back(Connector(&a, newAIndex, &b, newBIndex, inout));
			++wireCount;
		}
	}
	else
	{
		if (!aIn && bIn)
			connections.push_back(Connector(&a, aIndex, &b, bIndex, outin));
		else if (aIn && bIn)
			connectionsIn.push_back(Connector(&a, aIndex, &b, bIndex, inin));
		else if (!aIn && !bIn)
			connectionsOut.push_back(Connector(&a, aIndex, &b, bIndex, outout));
		else
			connectionsInOut.push_back(Connector(&a, aIndex, &b, bIndex, inout));

		++wireCount;
	}
}

void LogicSim::sortConnections()
{
	std::vector<Connector> tempIn;
	tempIn.reserve(connectionsIn.size());
	do
	{
		bool found = false;
		for (size_t i = 0; i < connectionsIn.size() - 1; ++i)
		{
			if (connectionsIn.back().a == connectionsIn[i].b && 
				connectionsIn.back().aIndex == connectionsIn[i].bIndex)
			{
				std::swap(connectionsIn.back(), connectionsIn[i]);
				found = true;
				break;
			}
		}
		if (!found)
		{
			tempIn.push_back(connectionsIn.back());
			connectionsIn.pop_back();
		}
	} while (connectionsIn.size());
	connectionsIn = tempIn;

	std::vector<Connector> tempOut;
	tempOut.reserve(connectionsOut.size());
	do
	{
		bool found = false;
		for (size_t i = 0; i < connectionsOut.size() - 1; ++i)
		{
			if (connectionsOut.back().a == connectionsOut[i].b && 
				connectionsOut.back().aIndex == connectionsOut[i].bIndex)
			{
				std::swap(connectionsOut.back(), connectionsOut[i]);
				found = true;
				break;
			}
		}
		if (!found)
		{
			tempOut.push_back(connectionsOut.back());
			connectionsOut.pop_back();
		}
	} while (connectionsOut.size());
	connectionsOut = tempOut;
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
	if (click)
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
		needsRecalc = true;
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

	needsRecalc = true;
}

void LogicSim::scroll(int x, int y)
{
	if (scale > 0)
	{
		x = x / (scale + 1.f) * 3 / 2.f;
		y = y / (scale + 1.f) * 3 / 2.f;
	}
	if (held)
	{
		int dx = -x;
		int dy = -y;
		(*held).setPos((*held).getRelPos().x + dx, (*held).getRelPos().y + dy);
	}
	setScroll(scrollx + x * (scale ? scale * 2 : 1), scrolly + y * (scale ? scale * 2 : 1));
}

LogicBlock* LogicSim::getHeld()
{
	return held;
}

void LogicSim::reset()
{
	board.reset();
}