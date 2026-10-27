#include "logicblock.h"

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
			board.connectToIn(delay, "in", temp, "in");
		else if (old)
			board.connect(*old, "out", temp, "in");

		old = &temp;
	}
	if (old)
		board.connectToOut(*old, "out", delay, "out");

	return delay;
}

// 1 cycle
static LogicBlock& createHalfAdd(Board& board, LogicBlock& parent)
{
	LogicBlock& add = parent.add(LogicBlock("a|b", "out|carry", "halfAdd"));

	LogicBlock& tempxor = add.add(LogicBlockXor);
	LogicBlock& tempand = add.add(LogicBlockAnd);

	board.connectToIn(add, "a", tempxor, "a");
	board.connectToIn(add, "b", tempxor, "b");
	board.connectToIn(add, "a", tempand, "a");
	board.connectToIn(add, "b", tempand, "b");

	board.connectToOut(tempxor, "out", add, "out");
	board.connectToOut(tempand, "out", add, "carry");

	return add;
}

// n cycles
static LogicBlock& createNBitAdd1(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& add = parent.add(LogicBlock(arrayToStr("a", n), 
		arrayToStr("out|overflow", n), std::to_string(n) + "bitAdd1"));

	LogicBlock* oldAdd = NULL;
	LogicBlock& one = add.add(LogicBlockOn);

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempAdd = createHalfAdd(board, add);
		board.connectToIn(add, arrayToStr("a", i), tempAdd, "a");

		if (i > 0)
			board.connect(*oldAdd, "carry", tempAdd, "b");
		else
			board.connect(one, "out", tempAdd, "b");

		board.connectToOut(tempAdd, "out", add, arrayToStr("out", i));

		oldAdd = &tempAdd;
	}
	if (oldAdd)
		board.connectToOut(*oldAdd, "carry", add, "overflow");

	return add;
}

// 3 cycles
static LogicBlock& createFullAdd(Board& board, LogicBlock& parent)
{
	LogicBlock& fullAdd = parent.add(LogicBlock("a|b|c", "out|carry", "fullAdd"));

	LogicBlock& tempxor0   = fullAdd.add(LogicBlockXor);
	LogicBlock& tempand0   = fullAdd.add(LogicBlockAnd);
	LogicBlock& tempxor1   = fullAdd.add(LogicBlockXor);
	LogicBlock& tempand1   = fullAdd.add(LogicBlockAnd);
	LogicBlock& tempor     = fullAdd.add(LogicBlockOr);
	LogicBlock& tempdelay0 = fullAdd.add(LogicBlockDelay);
	LogicBlock& tempdelay1 = fullAdd.add(LogicBlockDelay);
	LogicBlock& tempdelay2 = fullAdd.add(LogicBlockDelay);

	// step 0
	// tempxor0
	// tempand0
	// tempdelay2

	// step 1
	// tempdelay0
	// tempxor1
	// tempand1

	// step 2
	// tempdelay1
	// tempor

	board.connectToIn(fullAdd, "a", tempxor0, "a"); // 0
	board.connectToIn(fullAdd, "b", tempxor0, "b"); // 0

	board.connectToIn(fullAdd, "a", tempand0, "a"); // 0
	board.connectToIn(fullAdd, "b", tempand0, "b"); // 0
	board.connect(tempand0,    "out", tempdelay0, "in"); // 1

	board.connectToIn(fullAdd, "c", tempdelay2, "in"); // 0

	board.connect(tempxor0,        "out", tempxor1,   "a"); // 1
	board.connect(tempdelay2,      "out", tempxor1,   "b"); // 1
	board.connect(tempxor1,        "out", tempdelay1, "in"); // 2
	board.connectToOut(tempdelay1, "out", fullAdd,    "out"); // 3

	board.connect(tempxor0,   "out", tempand1, "a"); // 1
	board.connect(tempdelay2, "out", tempand1, "b"); // 1
	board.connect(tempand1,    "out", tempor,  "a"); // 2
	board.connect(tempdelay0,  "out", tempor,  "b"); // 2
	board.connectToOut(tempor, "out", fullAdd, "carry"); // 3

	return fullAdd;
}

// 2 cycles
static LogicBlock& createFullAddFast(Board& board, LogicBlock& parent)
{
	LogicBlock& fullAdd = parent.add(LogicBlock("a|b|c0|c1", "out|carry0|carry1", "fullAdd"));

	LogicBlock& tempxor0   = fullAdd.add(LogicBlockXor);
	LogicBlock& tempand0   = fullAdd.add(LogicBlockAnd);
	LogicBlock& tempxor1   = fullAdd.add(LogicBlockXor);
	LogicBlock& tempand1   = fullAdd.add(LogicBlockAnd);
	LogicBlock& tempor     = fullAdd.add(LogicBlockOr);
	LogicBlock& tempdelay0 = fullAdd.add(LogicBlockDelay);

	tempxor0.setPos(15, 15);
	tempand0.setPos(15, 45);
	tempor.setPos(15, 75);
	
	tempdelay0.setPos(65, 15);
	tempxor1.setPos(65, 45);
	tempand1.setPos(65, 75);

	// step 0
	// tempxor0
	// tempand0
	// tempor

	// step 1
	// tempdelay0
	// tempxor1
	// tempand1

	board.connectToIn(fullAdd, "a", tempxor0, "a"); // 0
	board.connectToIn(fullAdd, "b", tempxor0, "b"); // 0

	board.connectToIn(fullAdd, "a", tempand0, "a"); // 0
	board.connectToIn(fullAdd, "b", tempand0, "b"); // 0
	board.connect(tempand0, "out", tempdelay0, "in"); // 1

	board.connectToIn(fullAdd, "c0", tempor, "a"); // 0
	board.connectToIn(fullAdd, "c1", tempor, "b"); // 0

	board.connect(tempxor0, "out", tempxor1, "a"); // 1
	board.connect(tempor, "out", tempxor1, "b"); // 1
	board.connectToOut(tempxor1, "out", fullAdd, "out"); // 2

	board.connect(tempxor0, "out", tempand1, "a"); // 1
	board.connect(tempor, "out", tempand1, "b"); // 1
	board.connectToOut(tempand1, "out", fullAdd, "carry0"); // 2
	board.connectToOut(tempdelay0, "out", fullAdd, "carry1"); // 2

	return fullAdd;
}

static LogicBlock& createNBitNumber(Board& board, LogicBlock& parent, int n, int64_t input)
{
	LogicBlock& num = parent.add(LogicBlock("", arrayToStr("out", n), std::to_string(n) + "bitNumber"));

	for (int i = 0; i < n; ++i)
	{
		num.output[arrayToStr("out", i)] = (input & 1);
		input >>= 1;
	}

	return num;
}

// 2n + 1 cycles
static LogicBlock& createNBitAdd(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& add = parent.add(LogicBlock(
		arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitAdder"));

	LogicBlock* oldAdd = NULL;

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempAdd = createFullAddFast(board, add);
		tempAdd.setPos(15 + (i / (n / 4)) * 140, 15 + (i % (n / 4)) * 105);
		//tempAdd.setExpand(false);
		board.connectToIn(add, arrayToStr("a", i), tempAdd, "a");
		board.connectToIn(add, arrayToStr("b", i), tempAdd, "b");

		if (i > 0)
		{
			board.connect(*oldAdd, "carry0", tempAdd, "c0");
			board.connect(*oldAdd, "carry1", tempAdd, "c1");
		}

		board.connectToOut(tempAdd, "out", add, arrayToStr("out", i));

		oldAdd = &tempAdd;
	}

	if (oldAdd) // no one would make a 0bit adder
	{
		LogicBlock& tempor = add.add(LogicBlockOr);
		tempor.setPos(15 + (n / (n / 4)) * 140, 15 + (n % (n / 4)) * 50);
		board.connect(*oldAdd, "carry0", tempor, "a");
		board.connect(*oldAdd, "carry1", tempor, "b");
		board.connectToOut(tempor, "out", add, "overflow");
	}

	LogicBlock& delay = createNDelay(board, add, 2 * n + 1);
	delay.setPos(15 + ((n + 1) / (n / 4)) * 140, 15 + ((n + 1) % (n / 4)) * 50);
	//delay.setExpand(false);
	board.connectToIn(add, "sigin", delay, "in");
	board.connectToOut(delay, "out", add, "sigout");

	return add;
}

// 1 cycle
static LogicBlock& createNBitInvert(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& inv = parent.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n), std::to_string(n) + "bitInvert"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempNot = inv.add(LogicBlockNot);
		board.connectToIn(inv, arrayToStr("in", i), tempNot, "in");
		board.connectToOut(tempNot, "out", inv, arrayToStr("out", i));
	}

	return inv;
}

// 3n + 3 cycles
static LogicBlock& createNBitSub(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& sub = parent.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n),
		arrayToStr("out", n) + "|overflow", std::to_string(n) + "bitSubtract"));

	LogicBlock& inv = createNBitInvert(board, sub, n);
	LogicBlock& tempAdd = createNBitAdd1(board, sub, n);
	LogicBlock& add = createNBitAdd(board, sub, n);
	LogicBlock& ret = sub.add(LogicBlockNot);

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

	board.connectToIn(sub, "b[]", inv, "in[]"); // 0
	board.connect(inv, "out[]", tempAdd, "a[]"); // 1

	board.connectToIn(sub, "a[]", add, "a[]"); // 0
	board.connect(tempAdd, "out[]", add, "b[]"); // 2

	board.connectToOut(add, "out[]", sub, "out[]"); // 3
	board.connect(add, "overflow", ret, "in"); // 3
	board.connectToOut(ret, "out", sub, "overflow"); // 4

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
		board.connectToIn(nxor, arrayToStr("in", i), tempXor, "in");
		board.connectToOut(tempXor, "out", nxor, arrayToStr("out", i));
	}

	return nxor;
}

// log2(n) + 1 cycles
static LogicBlock& createNBitIsZero(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& iszero = parent.add(LogicBlock(arrayToStr("in", n), "out", 
		std::to_string(n) + "bitIsZero"));

	std::vector<LogicBlock*> ors;

	for (int i = 0; i < n; ++i)
	{
		if (i % 2 == 0)
		{
			LogicBlock& tempor = iszero.add(LogicBlockOr);
			ors.push_back(&tempor);
			board.connectToIn(iszero, arrayToStr("in", i), *ors.back(), "a");
		}
		else
			board.connectToIn(iszero, arrayToStr("in", i), *ors.back(), "b");
	}

	size_t len = (n + 1ull) / 2;
	size_t start = 0;
	while (len > 1)
	{
		for (int i = 0; i < len; ++i)
		{
			if (i % 2 == 0)
			{
				LogicBlock& tempor = iszero.add(LogicBlockOr);
				ors.push_back(&tempor);
				board.connect(*(ors[start + i]), "out", *ors.back(), "a");
			}
			else
				board.connect(*(ors[start + i]), "out", *ors.back(), "b");

		}
		len = (len + 1) / 2;
		start = ors.size() - len;
	}

	LogicBlock& tempnot = iszero.add(LogicBlockNot);
	board.connect(*(ors.back()), "out", tempnot, "in");
	board.connectToOut(tempnot, "out", iszero, "out");

	return iszero;
}

// log2(n) + 2 cycles
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

	board.connectToIn(comp, "a[]", nxor, "a[]");
	board.connectToIn(comp, "b[]", nxor, "b[]");

	board.connect(nxor, "out[]", iszero, "in[]");
	board.connectToOut(iszero, "out", comp, "out");

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
		board.connectToIn(gate, arrayToStr("in", i), tempand, "a");
		board.connectToIn(gate, "power", tempand, "b");
		board.connectToOut(tempand, "out", gate, arrayToStr("out", i));
	}

	return gate;
}

static LogicBlock& createNBitOutput(Board& board, LogicBlock& parent, int n)
{
	return parent.add(LogicBlock(arrayToStr("in", n), "", std::to_string(n) + "bitOutput"));
}

static int64_t readNBitOutput(const LogicBlock& block, int n)
{
	int64_t num = 0;
	for (int i = n - 1; i >= 0; --i)
	{
		num <<= 1ull;
		num |= (int64_t)block.input.at(arrayToStr("in", i));
	}
	return num;
}

int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("logsim", 1200, 600, 0);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	setPixelFormat(surface);

	Board board;

	int n = 16;
	LogicBlock& a = createNBitNumber(board, board.board, n, 0b1111111111111111);
	LogicBlock& b = createNBitNumber(board, board.board, n, 0b0000000000000001);
	LogicBlock& add = createNBitAdd(board, board.board, n);
	LogicBlock& out = createNBitOutput(board, board.board, n);
	LogicBlock& power = board.add(LogicBlockOn);
	a.setPos(15, 50);
	b.setPos(15, 200);
	add.setPos(100, 50);
	out.setPos(1000, 50);
	power.setPos(15, 350);
	
	board.connect(a, "out[]", add, "a[]");
	board.connect(b, "out[]", add, "b[]");
	board.connect(add, "out[]", out, "in[]");

	board.connect(power, "out", add, "sigin");

	board.board.recalc();
	
	int ticks = 0;
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
		drawTextFA(surface, 0, 600, -1, 1, 1, rgb(255, 255, 255), "Ticks: %d", ticks);
		SDL_UpdateWindowSurface(window);

		if (!add.output["sigout"])
		{
			board.tick();
			++ticks;
		}
		SDL_Delay(250);
	}
	SDL_DestroyWindow(window);

	//int i = 0;
	//for (; readNBitOutput(out, n) != 54321 + 12345; ++i)
	//	board.tick();
	//std::cout << i << '\n';
	//
	//sub.print();
	//std::cout << readNBitOutput(out, n) << '\n';

	//LogicBlock& a = createNBitNumber(board, board.board, 2, 0b11);
	//LogicBlock& b = createNBitNumber(board, board.board, 2, 0b11);
	//LogicBlock& add = createNBitAdd(board, board.board, 2);
	//LogicBlock& power = board.add(LogicBlockOn);
	//
	//board.connect(a, "out[]", add, "a[]");
	//board.connect(b, "out[]", add, "b[]");
	//board.connect(power, "out", add, "sigin");
	//
	//for (int i = 0; i < 65; ++i)
	//{
	//	board.tick();
	//	add.print();
	//}

	//LogicBlock& compare = createNBitCompare(board, 64);
	//LogicBlock& num0 = createNumberNBit(board, 45, 64);
	//LogicBlock& num1 = createNumberNBit(board, 45, 64);
	//board.connect(num0, "out[]", compare, "a[]");
	//board.connect(num1, "out[]", compare, "b[]");
	//for (int i = 0; i < 10; ++i)
	//	board.tick();
	//compare.print();

	SDL_Quit();

	return 0;
}