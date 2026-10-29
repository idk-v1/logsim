#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <list>
#include <map>

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

struct XYCoord
{
	XYCoord(int x = 0, int y = 0) : x(x), y(y) {}
	int x, y;
};

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

struct WriteData
{
	bool data = 0, written = 0;
};

struct LogicBlock
{
	LogicBlock() = default;
	LogicBlock(const std::string& label);
	LogicBlock(const std::string& in, const std::string& out, const std::string& label);

	void tick(int mousex, int mousey, bool click);

	LogicBlock& add(const LogicBlock& block);

	void print();

	void setPos(int x, int y);

	void setExpand(bool expand);

	void setExpandRec(bool expand);

	void recalc(int px = 0, int py = 0);

	void draw(SDL_Surface* surface);

	XYCoord getConnectionPos(int index, bool isInput);

	int getConnectionIndex(const std::string& name, bool isInput);

	void resetWrittenTo();

	bool isHidden() const;

	std::map<std::string, WriteData, strCmpWithArray> input;
	std::map<std::string, WriteData, strCmpWithArray> output;
	std::string label;

private:
	std::list<LogicBlock> blocks;

	int x = 0, y = 0, w = 0, h = 0, bx = 0, by = 0;
	bool expand = true, parExpand = true;
};

#define LogicBlockAnd    LogicBlock("and")
#define LogicBlockOr     LogicBlock("or")
#define LogicBlockNor    LogicBlock("nor")
#define LogicBlockNand   LogicBlock("nand")
#define LogicBlockXor    LogicBlock("xor")
#define LogicBlockNot    LogicBlock("not")
#define LogicBlockDelay  LogicBlock("delay")
#define LogicBlockOn     LogicBlock("on")
#define LogicBlockOff    LogicBlock("off")
#define LogicBlockToggle LogicBlock("toggle")