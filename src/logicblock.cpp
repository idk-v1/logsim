#include "logicblock.h"

#include "../graphics/graphics.h"
#include "../graphics/bulletinV1_font.h"

LogicBlock::LogicBlock(const std::string& label)
{
	if (label == "and" || label == "or" || label == "xor")
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
	else if (label == "off")
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
	else if (label == "not")
		output["out"].data = !input["in"].data;
	else if (label == "delay")
		output["out"].data = input["in"].data;
	else
		for (auto& b : blocks)
			b.tick();
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
	for (auto& b : blocks)
		b.setExpandRec(expand);
}

void LogicBlock::setExpandRec(bool expand)
{
	parExpand = expand;
}

void LogicBlock::recalc(int px, int py)
{

	int hfont_w = font_w / 2;
	int hfont_h = font_h / 2;

	bx = px + x + hfont_h;
	by = py + y + hfont_h;
	for (auto& b : blocks)
	{
		if (expand)
			b.recalc(bx, by);
	}

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
		maxLabel = std::max(maxLabel, 1 + inLen + outLen);
	}
	// fit name or at least 1 if blank
	w = hfont_w * (int)std::max(maxLabel, label.size());
	if (expand)
		for (auto& b : blocks)
			if (b.w)
				w = std::max(w, b.x + b.w);

	// 2*font_h for input/output on sides
	w += 2 * font_h;

	// fit max of input or output length or at least 1
	h = hfont_h * (int)std::max(1ull, std::max(input.size(), output.size()));
	if (expand)
		for (auto& b : blocks)
			if (b.h)
				h = std::max(h, b.y + b.h);

	// hfont_h for title
	h += hfont_h;
}

void LogicBlock::draw(SDL_Surface* surface)
{
	int hfont_h = font_h / 2;
	int hfont_w = font_w / 2;

	Uint32 on = rgb(255, 255, 0);
	Uint32 off = rgb(63, 63, 0);

	drawRect(surface, bx, by, w, h, rgb(31, 31, 31)); // bg
	drawRect(surface, bx, by, w, hfont_h, rgb(15, 15, 15)); // titlebar
	drawRect(surface, bx, by + hfont_h, hfont_h, h - hfont_h, rgb(63, 15, 15)); // inputs
	drawRect(surface, bx + w - hfont_h, by + hfont_h, hfont_h, h - hfont_h, rgb(15, 15, 63)); // outputs
	drawText(surface, bx + hfont_w, by, 0, rgb(255, 255, 255), label.data()); // title

	int index = 0;
	for (auto& i : input)
	{
		XYCoord xy = getConnectionPos(index, true);
		drawCircle(surface, xy.x, xy.y, hfont_w / 2,
			i.second.data ? on : off);
		drawTextA(surface, xy.x + hfont_w, xy.y, -1, 0, 0,
			rgb(255, 255, 255), i.first.data());
		++index;
	}
	index = 0;
	for (auto& i : output)
	{
		XYCoord xy = getConnectionPos(index, false);
		drawCircle(surface, xy.x, xy.y, hfont_w / 2,
			i.second.data ? on : off);
		drawTextA(surface, xy.x - hfont_w, xy.y, 1, 0, 0,
			rgb(255, 255, 255), i.first.data());

		++index;
	}

	if (expand)
		for (auto& b : blocks)
			b.draw(surface);

	drawRectOut(surface, bx - 1, by - 1, w + 2, h + 2, 1, rgb(127, 127, 127));
}

XYCoord LogicBlock::getConnectionPos(int index, bool isInput)
{
	if (index == -1)
		return XYCoord();

	int hfont_w = font_w / 2;
	int hfont_h = font_h / 2;

	int y = by + hfont_h + index * hfont_h;
	int x = bx;
	if (!isInput)
		x += w - hfont_h;

	if (x < 0)
		printf("bad\n");

	return XYCoord(x + hfont_h / 2, y + hfont_h / 2);
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
	printf("%s.%c[%s] doesn't exist", label.data(), isInput ? 'i' : 'o', name.data());
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

bool LogicBlock::isExpanded() const
{
	return parExpand;
}
