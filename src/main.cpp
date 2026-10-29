#include "logicblock.h"
#include "board.h"
#include "../graphics/graphics.h"
#include "../graphics/bulletinV1_font.h"

#include <iostream>

// n cycles
static LogicBlock& createNDelay(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& delay = parent.add(LogicBlock("in", "out", std::to_string(n) + "delay"));

	LogicBlock* old = NULL;
	for (int i = 0; i < n; ++i)
	{
		LogicBlock& temp = delay.add(LogicBlockDelay);
		temp.setPos(15 + (i / 10) * 50, 15 + (i % 10) * 30);
		if (i == 0)
			board.connect(delay, "in", temp, "in", inin);
		else if (old)
			board.connect(*old, "out", temp, "in");

		old = &temp;
	}
	if (old)
		board.connect(*old, "out", delay, "out", outout);

	return delay;
}

// 1 cycle
static LogicBlock& createHalfAdd(Board& board, LogicBlock& parent)
{
	LogicBlock& add = parent.add(LogicBlock("a|b", "out|carry", "halfAdd"));

	LogicBlock& tempxor = add.add(LogicBlockXor);
	LogicBlock& tempand = add.add(LogicBlockAnd);

	board.connect(add, "a", tempxor, "a", inin);
	board.connect(add, "b", tempxor, "b", inin);
	board.connect(add, "a", tempand, "a", inin);
	board.connect(add, "b", tempand, "b", inin);

	board.connect(tempxor, "out", add, "out", outout);
	board.connect(tempand, "out", add, "carry", outout);

	return add;
}

// n cycles
static LogicBlock& createNBitAdd1(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& add = parent.add(LogicBlock(arrayToStr("a", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitAdd1"));

	LogicBlock* oldAdd = NULL;
	LogicBlock& one = add.add(LogicBlockOn);

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempAdd = createHalfAdd(board, add);
		board.connect(add, arrayToStr("a", i), tempAdd, "a", inin);

		if (i > 0)
			board.connect(*oldAdd, "carry", tempAdd, "b");
		else
			board.connect(one, "out", tempAdd, "b");

		board.connect(tempAdd, "out", add, arrayToStr("out", i), outout);

		oldAdd = &tempAdd;
	}
	if (oldAdd)
		board.connect(*oldAdd, "carry", add, "overflow", outout);

	LogicBlock& delay = createNDelay(board, add, n);
	delay.setExpand(false);
	board.connect(add, "sigin", delay, "in", inin);
	board.connect(delay, "out", add, "sigout", outout);

	return add;
}

// 2 cycles
static LogicBlock& createFullAdd(Board& board, LogicBlock& parent)
{
	LogicBlock& fullAdd = parent.add(LogicBlock("a|b|c", "out|carry", "fullAdd"));

	LogicBlock& tempxor0   = fullAdd.add(LogicBlockXor);
	LogicBlock& tempand0   = fullAdd.add(LogicBlockAnd);
	LogicBlock& tempxor1   = fullAdd.add(LogicBlockXor);
	LogicBlock& tempand1   = fullAdd.add(LogicBlockAnd);
	LogicBlock& tempdelay0 = fullAdd.add(LogicBlockDelay);
	LogicBlock& tempdelay2 = fullAdd.add(LogicBlockDelay);

	// step 0
	// tempxor0
	// tempand0
	// tempdelay2

	// step 1
	// tempdelay0
	// tempxor1
	// tempand1

	board.connect(fullAdd, "a", tempxor0, "a", inin); // 0
	board.connect(fullAdd, "b", tempxor0, "b", inin); // 0

	board.connect(fullAdd,  "a", tempand0, "a", inin); // 0
	board.connect(fullAdd,  "b", tempand0, "b", inin); // 0
	board.connect(tempand0, "out", tempdelay0, "in"); // 1

	board.connect(fullAdd, "c", tempdelay2, "in", inin); // 0

	board.connect(tempxor0,   "out", tempxor1,   "a"); // 1
	board.connect(tempdelay2, "out", tempxor1,   "b"); // 1
	board.connect(tempxor1,   "out", fullAdd,    "out", outout); // 2

	board.connect(tempxor0,   "out", tempand1, "a"); // 1
	board.connect(tempdelay2, "out", tempand1, "b"); // 1
	board.connect(tempand1,    "out", fullAdd,  "carry", outout); // 2
	board.connect(tempdelay0,  "out", fullAdd,  "carry", outout); // 2

	return fullAdd;
}

static LogicBlock& createNBitNumber(Board& board, LogicBlock& parent, int n, int64_t input)
{
	LogicBlock& num = parent.add(LogicBlock("", arrayToStr("out", n), std::to_string(n) + "bitNumber"));

	bool neg = n < 0;
	for (int i = 0; i < n; ++i)
	{
		num.output[arrayToStr("out", i)].data = (input & 1);
		input >>= 1;
	}
	if (neg && n != 64)
		num.output[arrayToStr("out", n - 1)].data = neg;

	return num;
}

// 2n cycles
static LogicBlock& createNBitAdd(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& add = parent.add(LogicBlock(
		arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitAdder"));

	LogicBlock* oldAdd = NULL;

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempAdd = createFullAdd(board, add);
		tempAdd.setPos(15 + (i / (n / 4)) * 140, 15 + (i % (n / 4)) * 105);
		tempAdd.setExpand(false);
		board.connect(add, arrayToStr("a", i), tempAdd, "a", inin);
		board.connect(add, arrayToStr("b", i), tempAdd, "b", inin);

		if (i > 0)
		{
			board.connect(*oldAdd, "carry", tempAdd, "c");
		}

		board.connect(tempAdd, "out", add, arrayToStr("out", i), outout);

		oldAdd = &tempAdd;
	}

	if (oldAdd) // no one would make a 0bit adder
		board.connect(*oldAdd, "carry", add, "overflow", outout);

	LogicBlock& delay = createNDelay(board, add, 2 * n);
	delay.setPos(15 + ((n + 1) / (n / 4)) * 140, 15 + ((n + 1) % (n / 4)) * 50);
	delay.setExpand(false);
	board.connect(add, "sigin", delay, "in", inin);
	board.connect(delay, "out", add, "sigout", outout);

	return add;
}

// 1 cycle
static LogicBlock& createNBitInvert(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& inv = parent.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n), std::to_string(n) + "bitInvert"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempNot = inv.add(LogicBlockNot);
		board.connect(inv, arrayToStr("in", i), tempNot, "in", inin);
		board.connect(tempNot, "out", inv, arrayToStr("out", i), outout);
	}

	return inv;
}

// 3n + 2 cycles
static LogicBlock& createNBitSub(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& sub = parent.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitSubtract"));

	LogicBlock& inv = createNBitInvert(board, sub, n);
	LogicBlock& tempAdd = createNBitAdd1(board, sub, n);
	LogicBlock& add = createNBitAdd(board, sub, n);
	LogicBlock& ret = sub.add(LogicBlockNot);

	LogicBlock& delay0 = sub.add(LogicBlockDelay);
	delay0.setPos(15, 200);
	delay0.setExpand(false);

	inv.setExpand(false);
	inv.setPos(15, 100);
	tempAdd.setExpand(false);
	tempAdd.setPos(150, 100);
	add.setExpand(false);
	add.setPos(300, 15);
	ret.setPos(450, 100);

	// step 0
	// inv
	// delay0

	// step 1
	// tempAdd

	// step 2
	// add

	// step 3
	// ret
	// delay1

	board.connect(sub, "sigin", delay0, "in", inin);
	board.connect(delay0, "out", tempAdd, "sigin");
	board.connect(tempAdd, "sigout", add, "sigin");

	board.connect(sub, "b[]", inv, "in[]", inin); // 0
	board.connect(inv, "out[]", tempAdd, "a[]"); // 1

	board.connect(sub, "a[]", add, "a[]", inin); // 0
	board.connect(tempAdd, "out[]", add, "b[]"); // 2

	board.connect(add, "out[]", sub, "out[]", outout); // 3
	board.connect(add, "overflow", ret, "in"); // 3
	board.connect(ret, "out", sub, "overflow", outout); // 4

	board.connect(add, "sigout", sub, "sigout", outout);

	return sub;
}

// 1 cycle
static LogicBlock& createNBitXor(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& nxor = parent.add(LogicBlock(arrayToStr("in", n), 
		arrayToStr("out", n), std::to_string(n) + "bitXor"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempXor = nxor.add(LogicBlockXor);
		board.connect(nxor, arrayToStr("in", i), tempXor, "in", inin);
		board.connect(tempXor, "out", nxor, arrayToStr("out", i), outout);
	}

	return nxor;
}

// 1 cycle
static LogicBlock& createNBitIsZero(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& iszero = parent.add(LogicBlock(arrayToStr("in", n), "out", 
		std::to_string(n) + "bitIsZero"));

	LogicBlock& tempnot = iszero.add(LogicBlockNot);
	board.connect(tempnot, "out", iszero, "out", outout);

	for (int i = 0; i < n; ++i)
		board.connect(iszero, arrayToStr("in", i), tempnot, "in", inin);
	return iszero;
}

// 2 cycles
static LogicBlock& createNBitCompare(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& comp = parent.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n), 
		"out", std::to_string(n) + "bitCompare"));

	LogicBlock& nxor = createNBitXor(board, comp, n);
	LogicBlock& iszero = createNBitIsZero(board, comp, n);

	// step 0
	// nxor

	// step 1
	// iszero

	board.connect(comp, "a[]", nxor, "a[]", inin);
	board.connect(comp, "b[]", nxor, "b[]", inin);

	board.connect(nxor, "out[]", iszero, "in[]");
	board.connect(iszero, "out", comp, "out", outout);

	return comp;
}

// 1 cycle
static LogicBlock& createNBitGate(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& gate = parent.add(LogicBlock(arrayToStr("in", n) + "|power",
		arrayToStr("out", n), std::to_string(n) + "bitGate"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempand = gate.add(LogicBlockAnd);
		board.connect(gate, arrayToStr("in", i), tempand, "a", inin);
		board.connect(gate, "power", tempand, "b", inin);
		board.connect(tempand, "out", gate, arrayToStr("out", i), outout);
	}

	return gate;
}

// 1 cycle
static LogicBlock& createNBitShiftM(Board& board, LogicBlock& parent, int n, int m)
{
	LogicBlock& shift = parent.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n),
		std::to_string(n) + "bitShift" + (m > 0 ? "Up" : "Down") + std::to_string(abs(m))));

	for (int i = 0; i < n; ++i)
	{
		int index = i + m;
		if (index >= 0 && index < n)
		{
			board.connect(shift, arrayToStr("in", i), shift, arrayToStr("out", index), inout);
			//LogicBlock& delay = shift.add(LogicBlockDelay);
			//board.connectToIn(shift, arrayToStr("in", i), delay, "in");
			//board.connectToOut(delay, "out", shift, arrayToStr("out", index));
		}
	}

	return shift;
}

// n cycles on, n off
static LogicBlock& createClockN(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& clock = parent.add(LogicBlock("pause", "out", std::to_string(n) + "cycleClock"));

	LogicBlock& tempnot = clock.add(LogicBlockNot);
	tempnot.setPos(5, 5);
	LogicBlock* ptr = &tempnot;
	board.connect(tempnot, "out", clock, "out", outout);
	board.connect(clock, "pause", tempnot, "in", inin);

	for (int i = 1; i < n; ++i)
	{
		LogicBlock& delay = clock.add(LogicBlockDelay);
		delay.setPos(5, i * 25);
		board.connect(*ptr, "out", delay, "in");
		ptr = &delay;
	}

	board.connect(*ptr, "out", tempnot, "in");

	return clock;
}

static LogicBlock& createPulse(Board& board, LogicBlock& parent)
{
	LogicBlock& pulse = parent.add(LogicBlock("in", "out", "pulse"));

	LogicBlock& tempdelay = pulse.add(LogicBlockDelay);
	LogicBlock& tempxor = pulse.add(LogicBlockXor);
	LogicBlock& tempand = pulse.add(LogicBlockAnd);

	tempdelay.setPos(15, 15);
	tempxor.setPos(90, 15);
	tempand.setPos(165, 15);

	board.connect(pulse, "in", tempdelay, "in", inin);
	board.connect(pulse, "in", tempxor, "a", inin);
	board.connect(tempdelay, "out", tempxor, "b");
	board.connect(tempxor, "out", tempand, "a");
	board.connect(tempdelay, "out", tempand, "b");
	board.connect(tempand, "out", pulse, "out", outout);

	return pulse;
}

static LogicBlock& createNBitOutput(Board& board, LogicBlock& parent, int n)
{
	return parent.add(LogicBlock(arrayToStr("in", n), "", std::to_string(n) + "bitOutput"));
}

static int64_t readNBitOutput(const LogicBlock& block, int n)
{
	int64_t num = 0;
	bool neg = block.input.at(arrayToStr("in", n - 1)).data;
	for (int i = n - 1; i >= 0; --i)
	{
		num <<= 1ull;
		bool value = block.input.at(arrayToStr("in", i)).data;
		if (neg)
			value = !value;
		num |= (int64_t)value;
	}
	if (neg)
		num = -(num + 1);
	return num;
}

static int64_t readNBitInput(const LogicBlock& block, int n)
{
	int64_t num = 0;
	bool neg = block.output.at(arrayToStr("out", n - 1)).data;
	for (int i = n - 1; i >= 0; --i)
	{
		num <<= 1ull;
		bool value = block.output.at(arrayToStr("out", i)).data;
		if (neg)
			value = !value;
		num |= (int64_t)value;
	}
	if (neg)
		num = -(num + 1);
	return num;
}

static void drawNumberLabel(SDL_Surface* surface, LogicBlock& block, int n, bool isOutput)
{
	XYCoord coord = block.getConnectionPos(0, false);
	int64_t value = 0;
	if (isOutput)
		value = readNBitOutput(block, n);
	else
		value = readNBitInput(block, n);
	drawTextFA(surface, coord.x - font_w / 2, block.by - font_h / 2, 1, -1, 0, rgb(255, 255, 255), "%lld", value);
}

int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("logsim", 1200, 600, 0);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	setPixelFormat(surface);

	int stepTime = 500;

	Board board;

	//LogicBlock& clock1 = createClockN(board, board.board, 1);
	//clock1.setPos(5, 15);
	//LogicBlock& clock2 = createClockN(board, board.board, 2);
	//clock2.setPos(115, 15);
	//LogicBlock& clock3 = createClockN(board, board.board, 3);
	//clock3.setPos(225, 15);
	//LogicBlock& power = board.add(LogicBlockOn);
	//board.connect(power, "out", clock1, "pause");

	LogicBlock& clock = createClockN(board, board.board, 7);
	clock.setPos(15, 15);
	LogicBlock& pulse = createPulse(board, board.board);
	pulse.setPos(125, 15);

	board.connect(clock, "out", pulse, "in");

	//int n = 16;
	//LogicBlock& a = createNBitNumber(board, board.board, n, 10);
	//LogicBlock& b = createNBitNumber(board, board.board, n, 1);
	//LogicBlock& sub = createNBitSub(board, board.board, n);
	//LogicBlock& out = createNBitOutput(board, board.board, n);
	//LogicBlock& power = board.add(LogicBlockOn);
	//a.setPos(15, 50);
	//b.setPos(15, 150);
	//sub.setPos(100, 50);
	//out.setPos(1000, 50);
	//power.setPos(15, 350);
	//
	//board.connect(a, "out[]", sub, "a[]");
	//board.connect(b, "out[]", sub, "b[]");
	////board.connect(sub, "out[]", out, "in[]");
	//
	//board.connect(power, "out", sub, "sigin");
	//
	//LogicBlock& shift = createNBitShiftM(board, board.board, n, 1);
	//board.connect(sub, "out[]", shift, "in[]");
	//board.connect(shift, "out[]", out, "in[]");
	//shift.setPos(800, 50);

	//LogicBlock& clock = createClock(board, board.board);
	//clock.setPos(15, 15);

	//LogicBlock& iszero = createNBitIsZero(board, board.board, n);
	//board.connect(sub, "out[]", iszero, "in[]");
	//iszero.setPos(1000, 250);

	board.board.recalc();
	
	int ticks = 0;
	bool pause = false;
	bool running = true;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;
			}
		}
		clearScreen(surface, rgb(0, 0, 0));

		board.draw(surface);

		//drawNumberLabel(surface, a, n, false);
		//drawNumberLabel(surface, b, n, false);
		//drawNumberLabel(surface, out, n, true);
		
		drawTextFA(surface, 0, 600, -1, 1, 1, rgb(255, 255, 255), "Ticks: %d", ticks);

		SDL_UpdateWindowSurface(window);

		if (!pause)
		{
			board.tick();
			++ticks;
		}
		//if (sub.output["sigout"])
		//	pause = true;
		SDL_Delay(stepTime);
	}
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}