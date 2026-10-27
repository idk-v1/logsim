#pragma once
#include <vector>
#include <string>
#include <list>
#include <map>

#include "../graphics/graphics.h"
#include "../graphics/bulletinV1_font.h"

static std::string arrayToStr(const std::string& str, size_t i)
{
	return str + "[" + std::to_string(i) + "]";
}

// a[0] -> 0 | a[] -> -1
static int getArrayNumber(const std::string& str)
{
	size_t pos = str.find('[', 0);
	int num = 0;
	bool found = false;
	for (const char* c = str.data() + pos + 1; *c != ']'; ++c)
	{
		num = num * 10 + (*c - '0');
		found = true;
	}
	if (found)
		return num;
	return -1;
}

static bool isArray(const std::string& str)
{
	return str.find('[', 0) != std::string::npos;
}

struct strCmpWithArray
{
	bool operator()(const std::string& a, const std::string& b) const
	{
		if (isArray(a) && isArray(b))
		{
			for (const char* ca = a.data(), *cb = b.data(); *ca && *cb; ++ca, ++cb)
			{
				if (*ca == *cb)
				{
					if (*ca == '[')
					{
						int ai = getArrayNumber(a);
						int bi = getArrayNumber(b);
						return (ai < bi);
					}
				}
				else
					return (*ca < *cb);
			}
		}
		return a < b;
	}
};

struct LogicBlock
{
	std::map<std::string, bool, strCmpWithArray> input;
	std::map<std::string, bool, strCmpWithArray> output;
	std::list<LogicBlock> blocks;
	std::string label;

	int x = 0, y = 0, w = 0, h = 0;
	bool expand = true;

	LogicBlock() = default;
	LogicBlock(const std::string& label)
	{
		if (label == "and" || label == "or" || label == "xor")
		{
			input["a"] = 0;
			input["b"] = 0;
			output["out"] = 0;
		}
		else if (label == "not" || label == "delay")
		{
			input["in"] = 0;
			output["out"] = 0;
		}
		else if (label == "on")
			output["out"] = 1;
		else if (label == "off")
			output["out"] = 0;

		this->label = label;
	}
	LogicBlock(const std::string& in, const std::string& out, const std::string& label)
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
							input[arrayToStr(std::string(start, nameEnd - start), i)] = 0;
					}
					else
						input[std::string(start, nameEnd - start)] = 0;
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
							output[arrayToStr(std::string(start, nameEnd - start), i)] = 0;
					}
					else
						output[std::string(start, nameEnd - start)] = 0;
				}
				start = c + 1;

				if (*c == 0)
					break;
			}
		}

		this->label = label;
	}

	void tick()
	{
		if (label == "and")
			output["out"] = input["a"] && input["b"];
		else if (label == "or")
			output["out"] = input["a"] || input["b"];
		else if (label == "xor")
			output["out"] = input["a"] != input["b"];
		else if (label == "not")
			output["out"] = !input["in"];
		else if (label == "on")
			output["out"] = 1;
		else if (label == "off")
			output["out"] = 0;
		else if (label == "delay")
			output["out"] = input["in"];
		else
			for (auto& b : blocks)
				b.tick();
	}

	LogicBlock& add(const LogicBlock& block)
	{
		blocks.push_back(block);
		return blocks.back();
	}

	void print()
	{
		printf("%s\nIn:\n", label.data());
		for (auto& in : input)
			printf("[%s]: %d\n", in.first.data(), (int)in.second);
		printf("Out\n");
		for (auto& out : output)
			printf("[%s]: %d\n", out.first.data(), (int)out.second);
		printf("\n");
	}

	void setPos(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	void setExpand(bool expand)
	{
		this->expand = expand;
	}

	void recalc()
	{
		for (auto& b : blocks)
		{
			if (expand)
				b.recalc();
		}

		int hfont_w = font_w / 2;
		int hfont_h = font_h / 2;

		// fit name or at least 1 if blank
		w = hfont_w * std::max(1ull, label.size());
		if (expand)
		{
			for (auto& b : blocks)
			{
				if (b.w)
					if (b.x + b.w > w)
						w = b.x + b.w;
			}
		}
		// 2*font_w for input/output on sides
		w += font_w * 2;

		// fit max of input or output length or at least 1
		h = hfont_w * std::max(1ull, std::max(input.size(), output.size()));
		if (expand)
		{
			for (auto& b : blocks)
			{
				if (b.h)
					if (b.y + b.h > h)
						h = b.y + b.h;
			}
		}
		// hfont_h for title
		h += hfont_h;
	}

	void draw(SDL_Surface* surface, int px, int py)
	{
		int bx = px + x;
		int by = py + y;

		int hfont_h = font_h / 2;
		int hfont_w = font_w / 2;

		drawRect(surface, bx, by, w, h, rgb(31, 31, 31)); // bg
		drawRect(surface, bx, by, w, hfont_h, rgb(15, 15, 15)); // titlebar
		drawRect(surface, bx, by + hfont_h, hfont_w, h - hfont_h, rgb(63, 15, 15)); // inputs
		drawRect(surface, bx + w - hfont_w, by + hfont_h, hfont_w, h - hfont_h, rgb(15, 15, 63)); // outputs
		drawText(surface, bx + hfont_w, by, 0, rgb(255, 255, 255), label.data()); // title
		
		int off = by + hfont_h;
		for (auto& i : input)
		{
			drawCircleA(surface, bx, off, -1, -1, hfont_w / 2, 
				i.second ? rgb(255, 255, 0) : rgb(63, 63, 0));
			off += hfont_w;
		}
		off = by + hfont_h;
		for (auto& i : output)
		{
			drawCircleA(surface, bx + w - hfont_w, off, -1, -1, hfont_w / 2,
				i.second ? rgb(255, 255, 0) : rgb(63, 63, 0));
			off += hfont_w;
		}

		if (expand)
			for (auto& b : blocks)
				b.draw(surface, bx, by);

		drawRectOut(surface, bx - 1, by - 1, w + 2, h + 2, 1, rgb(127, 127, 127));
	}
};

#define LogicBlockAnd   LogicBlock("and")
#define LogicBlockOr    LogicBlock("or")
#define LogicBlockXor   LogicBlock("xor")
#define LogicBlockNot   LogicBlock("not")
#define LogicBlockDelay LogicBlock("delay")
#define LogicBlockOn    LogicBlock("on")
#define LogicBlockOff   LogicBlock("off")


struct Connector
{
	Connector(LogicBlock& a, const std::string& aOut, LogicBlock& b, const std::string& bIn, int mode) :
		a(a), b(b), aOut(aOut), bIn(bIn), mode(mode) {}

	void tick()
	{
		std::map<std::string, bool, strCmpWithArray>* aport = NULL;
		std::map<std::string, bool, strCmpWithArray>* bport = NULL;
		if (mode == 0)
		{
			aport = &a.output;
			bport = &b.input;
		}
		else if (mode == 1)
		{
			aport = &a.input;
			bport = &b.input;
		}
		else if (mode == -1)
		{
			aport = &a.output;
			bport = &b.output;
		}

		if (isArray(aOut) && isArray(bIn) && getArrayNumber(aOut) == -1 && getArrayNumber(bIn) == -1)
		{
			std::string aOut2(aOut, 0, aOut.size() - 2);
			std::string bIn2(bIn, 0, bIn.size() - 2);
			// copy all
			for (int i = 0;; ++i)
			{
				if (aport->find(arrayToStr(aOut2, i)) != aport->end() &&
					bport->find(arrayToStr(bIn2, i)) != bport->end())
					(*bport)[arrayToStr(bIn2, i)] = (*aport)[arrayToStr(aOut2, i)];
				else
					break;
			}
		}
		else // could have done a zero out, then or into outputs if i was smarter
			(*bport)[bIn] = (*aport)[aOut];
	}

	LogicBlock& a, &b;
	std::string aOut, bIn;
	int mode;
};

struct Board
{
	LogicBlock board;
	std::vector<Connector> connections;
	std::vector<Connector> connectionsIn;
	std::vector<Connector> connectionsOut;

	Board()
	{
		board.label = "Board";
	}

	// a.out -> b.in
	void connect(LogicBlock& a, const std::string& aOut, LogicBlock& b, const std::string& bIn)
	{
		connections.push_back(Connector(a, aOut, b, bIn, 0));
	}

	// a.in -> a.child.in
	void connectToIn(LogicBlock& a, const std::string& aIn, LogicBlock& b, const std::string& bIn)
	{
		connectionsIn.push_back(Connector(a, aIn, b, bIn, 1));
	}

	// a.child.out -> a.out
	void connectToOut(LogicBlock& a, const std::string& aOut, LogicBlock& b, const std::string& bOut)
	{
		connectionsOut.push_back(Connector(a, aOut, b, bOut, -1));
	}

	LogicBlock& add(const LogicBlock& block)
	{
		return board.add(block);
	}

	void tick()
	{
		for (auto& c : connections)
			c.tick();

		for (auto& c : connectionsIn)
			c.tick();

		board.tick();

		for (auto& c : connectionsOut)
			c.tick();
	}

	void draw(SDL_Surface* surface)
	{
		board.draw(surface, 0, 0);
	}
};