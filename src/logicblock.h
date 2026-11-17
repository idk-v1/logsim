#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <list>
#include <vector>

static std::string arrayToStr(const std::string& str, size_t i)
{
	return str + "[" + std::to_string(i) + "]";
}

static std::string arrayToName(const std::string& str)
{
	size_t pos = str.find('[');
	if (pos != std::string::npos)
		return str.substr(0, pos);
	return str;
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

struct WriteData
{
	bool data = 0, written = 0;
};

struct LogicBlock
{
	LogicBlock() = default;
	LogicBlock(const std::string& label);
	LogicBlock(const std::string& in, const std::string& out, const std::string& label);

	void tick();
	void interact(int mousex, int mousey, int oldmousex, int oldmousey,
		bool click, bool oldclick, bool& found, LogicBlock** held);

	LogicBlock& add(const LogicBlock& block);

	void setPos(int x, int y);

	void setExpand(bool expand);

	void recalc(int px, int py, bool expanded);

	void draw(SDL_Surface* surface, LogicBlock* held);

	XYCoord getConnectionPos(int index, bool isInput);

	int getConnectionIndex(const std::string& name, bool isInput);

	void resetWrittenTo();

	bool isHidden() const;
	bool isExpanded() const;

	XYCoord getPos() const;
	XYCoord getRelPos() const;
	XYCoord getSize() const;

	std::vector<WriteData> input;
	std::vector<WriteData> output;
	std::vector<std::string> inputLabels;
	std::vector<std::string> outputLabels;
	std::string label;

	static void setFontSize(int w, int h, int fontsize, int posMul);

private:
	std::list<LogicBlock> blocks;

	static int fontw, fonth, fontsize, posMul;

	int x = 5, y = 5, w = 0, h = 0, bx = 0, by = 0;
	bool expand = false, hidden = false;
};

// in: a, b
// out: out
#define LogicBlockAnd    LogicBlock("and")
// in: a, b
// out: out
#define LogicBlockOr     LogicBlock("or")
// in: a, b
// out: out
#define LogicBlockNor    LogicBlock("nor")
// in: a, b
// out: out
#define LogicBlockNand   LogicBlock("nand")
// in: a, b
// out: out
#define LogicBlockXnor   LogicBlock("xnor")
// in: a, b
// out: out
#define LogicBlockXor    LogicBlock("xor")
// in: in
// out: out
#define LogicBlockNot    LogicBlock("not")
// in: in
// out: out
#define LogicBlockDelay  LogicBlock("delay")
// out: out
#define LogicBlockOn     LogicBlock("on")
// out: out
#define LogicBlockOff    LogicBlock("off")
// out: out
#define LogicBlockToggle LogicBlock("toggle")
// out: out
#define LogicBlockButton LogicBlock("button")