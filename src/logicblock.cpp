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
		input["a"] = { 0 };
		input["b"] = { 0 };
		output["out"] = { 0 };
	}
	else if (label == "not" || label == "delay")
	{
		input["in"] = { 0 };
		output["out"] = { 0 };
	}
	else if (label == "on")
		output["out"] = { 1, 0 };
	else if (label == "off" || label == "toggle" || label == "button")
		output["out"] = { 0 };

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
						input[arrayToStr(std::string(start, nameEnd - start), i)] = { 0 };
				}
				else
					input[std::string(start, nameEnd - start)] = { 0 };
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
						output[arrayToStr(std::string(start, nameEnd - start), i)] = { 0 };
				}
				else
					output[std::string(start, nameEnd - start)] = { 0 };
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
		output["out"].data = input["a"].data && input["b"].data;
	else if (label == "or")
		output["out"].data = input["a"].data || input["b"].data;
	else if (label == "xor")
		output["out"].data = input["a"].data != input["b"].data;
	else if (label == "nor")
		output["out"].data = !(input["a"].data || input["b"].data);
	else if (label == "nand")
		output["out"].data = !(input["a"].data && input["b"].data);
	else if (label == "xnor")
		output["out"].data = !(input["a"].data != input["b"].data);
	else if (label == "not")
		output["out"].data = !input["in"].data;
	else if (label == "delay")
		output["out"].data = input["in"].data;
	else
		for (auto& b : blocks)
			b.tick();
}

void LogicBlock::interact(int mousex, int mousey, int oldmousex, int oldmousey, bool click, bool oldclick, bool& found, LogicBlock** held)
{
	for (auto& b : blocks)
		b.interact(mousex, mousey, oldmousex, oldmousey, click, oldclick, found, held);

	bool clicked = (expanded && mousex >= bx && mousex <= bx + w && mousey >= by + fonth &&
		mousey <= by + h && click && !oldclick);

	if (expanded && click)
	{
		if (!*held && mousex >= bx && mousex <= bx + w && mousey >= by &&
			mousey <= by + fonth && !found)
		{
			x += (mousex - oldmousex) / posMul;
			y += (mousey - oldmousey) / posMul;
			x = std::max(5, x);
			y = std::max(5, y);
			found = true;
			*held = this;
		}
		else if (*held == this)
		{
			x += (mousex - oldmousex) / posMul;
			y += (mousey - oldmousey) / posMul;
			x = std::max(5, x);
			y = std::max(5, y);
			found = true;
			*held = this;
		}
	}

	if (label == "toggle")
	{
		if (clicked)
			output["out"].data = !output["out"].data;
	}
	else if (label == "button")
	{
		if (clicked)
			output["out"].data = true;
		else
			output["out"].data = false;
	}
}

LogicBlock& LogicBlock::add(const LogicBlock& block)
{
	blocks.push_back(block);
	return blocks.back();
}

void LogicBlock::print()
{
	printf("%s\nIn:\n", label.data());
	for (auto& in : input)
		printf("[%s]: %d\n", in.first.data(), (int)in.second.data);
	printf("Out\n");
	for (auto& out : output)
		printf("[%s]: %d\n", out.first.data(), (int)out.second.data);
	printf("\n");
}

void LogicBlock::setPos(int x, int y)
{
	this->x = x;
	this->y = y;
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
		this->expanded = false;
		for (auto& b : blocks)
			b.recalc(bx, by, false);
		return;
	}

	this->expanded = true;

	bx = px + x * posMul + fonth;
	by = py + y * posMul + fontw;
	for (auto& b : blocks)
		b.recalc(bx, by, expand);

	size_t maxLabel = 1;
	auto in = input.begin();
	auto out = output.begin();
	for (int i = 0; i < input.size() || i < output.size(); ++i)
	{
		size_t inLen = 0;
		size_t outLen = 0;

		if (in != input.end())
		{
			inLen = in->first.size();
			in = std::next(in);
		}
		if (out != output.end())
		{
			outLen = out->first.size();
			out = std::next(out);
		}
		maxLabel = std::max(maxLabel, 2 + inLen + outLen);
	}
	// fit name or at least 1 if blank
	w = fontw * (int)std::max(maxLabel, label.size());
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

	if (label == "on" || (label == "toggle" && output["out"].data))
		drawRect(surface, bx, by, w, h, rgb(63, 63, 63)); // bg
	else
		drawRect(surface, bx, by, w, h, rgb(31, 31, 31)); // bg
	drawRect(surface, bx, by, w, fonth, rgb(15, 15, 15)); // titlebar
	drawRect(surface, bx, by + fonth, fontw, h - fonth, rgb(63, 15, 15)); // inputs
	drawRect(surface, bx + w - fontw, by + fonth, fontw, h - fonth, rgb(15, 15, 63)); // outputs
	drawText(surface, bx + fontw, by, fontsize, rgb(255, 255, 255), label.data()); // title

	int index = 0;
	for (auto& i : input)
	{
		XYCoord xy = getConnectionPos(index, true);
		drawCircle(surface, xy.x, xy.y, fontw / 2,
			i.second.data ? on : off);
		drawTextA(surface, xy.x + fontw, xy.y, -1, 0, fontsize,
			rgb(255, 255, 255), i.first.data());
		++index;
	}
	index = 0;
	for (auto& i : output)
	{
		XYCoord xy = getConnectionPos(index, false);
		drawCircle(surface, xy.x, xy.y, fontw / 2,
			i.second.data ? on : off);
		drawTextA(surface, xy.x - fontw, xy.y, 1, 0, fontsize,
			rgb(255, 255, 255), i.first.data());

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
		for (auto& i : input)
		{
			if (i.first == name)
				return index;
			++index;
		}
	}
	else
	{
		for (auto& i : output)
		{
			if (i.first == name)
				return index;
			++index;
		}
	}
	printf("%s.%c[%s] doesn't exist\n", label.data(), isInput ? 'i' : 'o', name.data());
	return -1;
}

void LogicBlock::resetWrittenTo()
{
	for (auto& b : blocks)
		b.resetWrittenTo();

	for (auto& i : input)
		i.second.written = false;

	for (auto& o : output)
		o.second.written = false;
}

bool LogicBlock::isHidden() const
{
	return !expanded;
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
