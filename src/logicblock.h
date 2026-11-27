#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <list>
#include <vector>

struct LogicTemplate
{
	LogicTemplate(const std::string& in, const std::string& out, const std::string& label);

	std::vector<std::string> input, output;
	std::string name;
	int count;
};

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
	XYCoord(float x = 0, float y = 0) : x(x), y(y) {}
	float x, y;
};

struct WriteData
{
	bool data = 0, written = 0;
};

struct LogicBlock
{
	LogicBlock() = default;
	LogicBlock(const std::string& in, const std::string& out, const std::string& label, bool exclReset = false);

	void tick();
	void interact(int mousex, int mousey, int oldmousex, int oldmousey,
		bool click, bool oldclick, bool& found, LogicBlock** held);

	LogicBlock& add(const LogicBlock& block);

	void setPos(float x, float y);

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

	void reset();

	std::vector<WriteData> input;
	std::vector<WriteData> output;
	//WriteData* input = NULL;
	//WriteData* output = NULL;
	//int inputSize = 0;
	//int outputSize = 0;

	static void setFontSize(int w, int h, int fontsize, int posMul);

	std::string getLabel();

	static size_t blockCount;

private:
	std::list<LogicBlock> blocks;

	static std::vector<LogicTemplate> templs;
	int findTempl(const std::string& name);

	static int fontw, fonth, fontsize, posMul;

	int label = -1;

	float x = 5, y = 5;
	int w = 0, h = 0, bx = 0, by = 0;
	bool expand = false, hidden = false;
	bool excludeReset = false;
};

// in: a, b
// out: out
// index 0
#define LogicBlockAnd    LogicBlock("a|b", "out", "and")
// in: a, b
// out: out
// index 1
#define LogicBlockOr     LogicBlock("a|b", "out", "or")
// in: a, b
// out: out
// index 2
#define LogicBlockNor    LogicBlock("a|b", "out", "nor")
// in: a, b
// out: out
// index 3
#define LogicBlockNand   LogicBlock("a|b", "out", "nand")
// in: a, b
// out: out
// index 4
#define LogicBlockXnor   LogicBlock("a|b", "out", "xnor")
// in: a, b
// out: out
// index 5
#define LogicBlockXor    LogicBlock("a|b", "out", "xor")
// in: in
// out: out
// index 6
#define LogicBlockNot    LogicBlock("in", "out", "not")
// in: in
// out: out
// index 7
#define LogicBlockDelay  LogicBlock("in", "out", "delay")
// out: out
// index 8
#define LogicBlockOn     LogicBlock("", "out", "on", true)
// out: out
// index 9
#define LogicBlockOff    LogicBlock("", "out", "off", true)
// out: out
// index 10
#define LogicBlockToggle LogicBlock("", "out", "toggle")
// out: out
// index 11
#define LogicBlockButton LogicBlock("", "out", "button")
// in: in
// index 12
#define LogicBlockLED    LogicBlock("in", "out", "led")

enum BuiltInBlocks
{
	And,
	Or,
	Nor,
	Nand,
	Xnor,
	Xor,
	Not,
	Delay,
	On,
	Off,
	Toggle,
	Button,
	LED,
	Max
};