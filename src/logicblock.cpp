#include "logicblock.h"

#include "../graphics/graphics.h"
#include "../graphics/bulletinV1_font.h"

int LogicBlock::fontw = 0;
int LogicBlock::fonth = 0;
int LogicBlock::fontsize = 0;
int LogicBlock::posMul = 1;

LogicBlock::LogicBlock(const std::string& label)
{
	if (label == "and" || label == "or" || label == "xor" || label == "nor" || 
		label == "nand" || label == "xnor")
	{
		input = { {0}, {0} };
		inputLabels = {"a", "b"};
		output = { {0} };
		outputLabels = {"out"};
	}
	else if (label == "not" || label == "delay")
	{
		input = { {0} };
		inputLabels = { "in" };
		output = { {0} };
		outputLabels = { "out" };
	}
	else if (label == "on")
	{
		output = { {1, 0} };
		outputLabels = { "out" };
	}
	else if (label == "off" || label == "toggle" || label == "button")
	{
		output = { {0} };
		outputLabels = { "out" };
	}

	this->label = label;
}

LogicBlock::LogicBlock(const std::string& in, const std::string& out, const std::string& label)
{
	const char* start = in.data();
	for (const char* c = start;; ++c)
	{
		if (*c == '|' || *c == 0)
		{
			if (c - start > 0)
			{
				bool inArray = false;
				int arraySize = 0;
				const char* nameEnd = c;
				for (const char* ch = start; ch != c; ++ch)
				{
					if (*ch == '[')
					{
						inArray = true;
						nameEnd = ch;
					}
					else if (*ch == ']')
						inArray = false;
					else if (inArray)
						arraySize = arraySize * 10 + (*ch - '0');
				}
				if (arraySize)
				{
					for (int i = 0; i < arraySize; ++i)
					{
						input.push_back({ 0 });
						inputLabels.push_back(arrayToStr(std::string(start, nameEnd - start), i));
					}
				}
				else
				{
					input.push_back({ 0 });
					inputLabels.push_back(std::string(start, nameEnd - start));
				}
			}
			start = c + 1;

			if (*c == 0)
				break;
		}
	}

	start = out.data();
	for (const char* c = start;; ++c)
	{
		if (*c == '|' || *c == 0)
		{
			if (c - start > 0)
			{
				bool inArray = false;
				int arraySize = 0;
				const char* nameEnd = c;
				for (const char* ch = start; ch != c; ++ch)
				{
					if (*ch == '[')
					{
						inArray = true;
						nameEnd = ch;
					}
					else if (*ch == ']')
						inArray = false;
					else if (inArray)
						arraySize = arraySize * 10 + (*ch - '0');
				}
				if (arraySize)
				{
					for (int i = 0; i < arraySize; ++i)
					{
						output.push_back({0});
						outputLabels.push_back(arrayToStr(std::string(start, nameEnd - start), i));
					}
				}
				else
				{
					output.push_back({0});
					outputLabels.push_back(std::string(start, nameEnd - start));
				}
			}
			start = c + 1;

			if (*c == 0)
				break;
		}
	}

	this->label = label;
}

void LogicBlock::tick()
{
	if (label == "and")
		output[0].data = input[0].data && input[1].data;
	else if (label == "or")
		output[0].data = input[0].data || input[1].data;
	else if (label == "xor")
		output[0].data = input[0].data != input[1].data;
	else if (label == "nor")
		output[0].data = !(input[0].data || input[1].data);
	else if (label == "nand")
		output[0].data = !(input[0].data && input[1].data);
	else if (label == "xnor")
		output[0].data = !(input[0].data != input[1].data);
	else if (label == "not")
		output[0].data = !input[0].data;
	else if (label == "delay")
		output[0].data = input[0].data;
	else if (label == "button")
		output[0].data = 0;
	else
		for (auto& b : blocks)
			b.tick();
}

void LogicBlock::interact(int mousex, int mousey, int oldmousex, int oldmousey, bool click, bool oldclick, bool& found, LogicBlock** held)
{
	for (auto& b : blocks)
		b.interact(mousex, mousey, oldmousex, oldmousey, click, oldclick, found, held);

	bool mouseOverWhole = (!hidden && mousex >= bx && mousex <= bx + w && 
		mousey >= by &&	mousey <= by + h);
	bool mouseOverBody = mouseOverWhole && (mousey >= by + fonth);
	bool mouseOverTitle = mouseOverWhole && (mousey <= by + fonth);
	bool mouseOverExpand = mouseOverTitle && (mousex >= bx + w - fontw);

	bool clickOnce = click && !oldclick;
	bool clickHold = click;

	if (clickHold)
	{
		if ((!*held && !found && mouseOverTitle) || *held == this)
		{
			x += (mousex - oldmousex) / posMul;
			y += (mousey - oldmousey) / posMul;
			x = std::max(5, x);
			y = std::max(5, y);
			found = true;
			*held = this;
		}
	}

	if (mouseOverExpand && clickOnce && blocks.size())
	{
		setExpand(!expand);
	}

	if (label == "toggle")
	{
		if (mouseOverBody && clickOnce)
			output[0].data = !output[0].data;
	}
	else if (label == "button")
		output[0].data |= (mouseOverBody && clickOnce);
}

LogicBlock& LogicBlock::add(const LogicBlock& block)
{
	blocks.push_back(block);
	return blocks.back();
}

void LogicBlock::setPos(int x, int y)
{
	this->x = std::max(x, 5);
	this->y = std::max(y, 5);
}

void LogicBlock::setExpand(bool expand)
{
	this->expand = expand;
}

void LogicBlock::recalc(int px, int py, bool expanded)
{
	if (!expanded)
	{
		w = 0;
		h = 0;
		bx = 0;
		by = 0;
		this->hidden = true;
		for (auto& b : blocks)
			b.recalc(bx, by, false);
		return;
	}

	this->hidden = false;

	bx = px + x * posMul + fonth;
	by = py + y * posMul + fontw;
	for (auto& b : blocks)
		b.recalc(bx, by, expand);

	uint32_t maxLabel = 1;
	for (int i = 0; i < input.size() || i < output.size(); ++i)
	{
		uint32_t inLen = 0;
		uint32_t outLen = 0;

		if (i < input.size())
		{
			getTextSize(inputLabels[i].data(), fontsize, &inLen, NULL);
		}
		if (i < output.size())
		{
			getTextSize(outputLabels[i].data(), fontsize, &outLen, NULL);
		}
		maxLabel = std::max(maxLabel, 2 * fontw + inLen + outLen);
	}
	// fit name or at least 1 if blank
	uint32_t labelw = 0;
	getTextSize(label.data(), fontsize, &labelw, NULL);
	w = (int)std::max(maxLabel, labelw);
	if (expand)
		for (auto& b : blocks)
			if (b.w)
				w = std::max(w, b.x * posMul + b.w);

	// 2*font_h for input/output on sides
	w += 4 * fontw;

	// fit max of input or output length or at least 1
	h = (fonth - fontw) + fonth * (int)std::max(1ull, std::max(input.size(), output.size()));
	if (expand)
		for (auto& b : blocks)
			if (b.h)
				h = std::max(h, b.y * posMul + b.h);

	// hfont_h for title
	h += fonth;
}

void LogicBlock::draw(SDL_Surface* surface, LogicBlock* held)
{
	Uint32 on = rgb(255, 255, 0);
	Uint32 off = rgb(63, 63, 0);

	if (label == "on" || (label == "toggle" && output[0].data))
		drawRect(surface, bx, by, w, h, rgb(63, 63, 63)); // bg
	else
		drawRect(surface, bx, by, w, h, rgb(31, 31, 31)); // bg
	drawRect(surface, bx, by, w, fonth, rgb(15, 15, 15)); // titlebar
	drawRect(surface, bx, by + fonth, fontw, h - fonth, rgb(63, 15, 15)); // inputs
	drawRect(surface, bx + w - fontw, by + fonth, fontw, h - fonth, rgb(15, 15, 63)); // outputs
	if (blocks.size())
	{
		drawRect(surface, bx + w - fontw, by, fontw, fonth, rgb(95, 95, 95));
		drawText(surface, bx + w - fontw, by, fontsize, rgb(255, 255, 255), expand ? "-" : "+");
	}
	drawText(surface, bx + fontw, by, fontsize, rgb(255, 255, 255), label.data()); // title

	int index = 0;
	for (int i = 0; i < input.size(); ++i)
	{
		XYCoord xy = getConnectionPos(index, true);
		drawCircle(surface, xy.x, xy.y, fontw / 2,
			input[i].data ? on : off);
		drawTextA(surface, xy.x + fontw, xy.y, -1, 0, fontsize,
			rgb(255, 255, 255), inputLabels[i].data());
		++index;
	}
	index = 0;
	for (int i = 0; i < output.size(); ++i)
	{
		XYCoord xy = getConnectionPos(index, false);
		drawCircle(surface, xy.x, xy.y, fontw / 2,
			output[i].data ? on : off);
		drawTextA(surface, xy.x - fontw, xy.y, 1, 0, fontsize,
			rgb(255, 255, 255), outputLabels[i].data());

		++index;
	}

	if (expand)
		for (auto& b : blocks)
			b.draw(surface, held);

	drawRectOut(surface, bx - 1, by - 1, w + 2, h + 2, 1, 
		held == this ? rgb(127, 255, 31) : rgb(127, 127, 127));
}

XYCoord LogicBlock::getConnectionPos(int index, bool isInput)
{
	if (index == -1)
		return XYCoord();

	int y = by + (fonth - fontw) + (index + 1) * fonth;
	int x = bx;
	if (!isInput)
		x += w - fontw;

	return XYCoord(x + fontw / 2, y + fontw / 2);
}

int LogicBlock::getConnectionIndex(const std::string& name, bool isInput)
{
	int index = 0;
	if (isInput)
	{
		for (int i = 0; i < input.size(); ++i)
		{
			if (inputLabels[i] == name)
				return index;
			++index;
		}
	}
	else
	{
		for (int i = 0; i < output.size(); ++i)
		{
			if (outputLabels[i] == name)
				return index;
			++index;
		}
	}
	return -1;
}

void LogicBlock::resetWrittenTo()
{
	for (auto& b : blocks)
		b.resetWrittenTo();

	for (auto& i : input)
		i.written = false;

	for (auto& o : output)
		o.written = false;
}

bool LogicBlock::isHidden() const
{
	return hidden;
}

bool LogicBlock::isExpanded() const
{
	return expand;
}

XYCoord LogicBlock::getPos() const
{
	return XYCoord(bx, by);
}

XYCoord LogicBlock::getRelPos() const
{
	return XYCoord(x, y);
}

XYCoord LogicBlock::getSize() const
{
	return XYCoord(w, h);
}

void LogicBlock::setFontSize(int w, int h, int fontsize, int posMul)
{
	fontw = w;
	fonth = h;
	LogicBlock::fontsize = fontsize;
	LogicBlock::posMul = posMul;
}
