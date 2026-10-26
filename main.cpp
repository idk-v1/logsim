#include "logicblock.h"

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

static LogicBlock& createNumberNBit(Board& board, LogicBlock& parent, int n, int64_t input)
{
	LogicBlock& num = parent.add(LogicBlock("", arrayToStr("out", n), std::to_string(n) + "bitNumber"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& temp = num.add((input & 1) ? LogicBlockOn : LogicBlockOff);
		board.connectToOut(temp, "out", num, arrayToStr("out", i));
		input >>= 1;
	}

	return num;
}

static LogicBlock& createNBitAdd(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& add = parent.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitAdder"));

	LogicBlock* oldAdd = NULL;

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempAdd = createFullAddFast(board, add);
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
		board.connect(*oldAdd, "carry0", tempor, "a");
		board.connect(*oldAdd, "carry1", tempor, "b");
		board.connectToOut(tempor, "out", add, "overflow");
	}

	LogicBlock* tempDelay = NULL;
	for (int i = 0; i < n * 2 + 1; ++i)
	{
		LogicBlock& delay = add.add(LogicBlockDelay);

		if (i == 0)
			board.connectToIn(add, "sigin", delay, "in");
		else
			board.connect(*tempDelay, "out", delay, "in");

		tempDelay = &delay;
	}
	if (tempDelay)
		board.connectToOut(*tempDelay, "out", add, "sigout");

	return add;
}

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

static LogicBlock& createNBitSub(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& sub = parent.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitSubtract"));

	LogicBlock& inv = createNBitInvert(board, sub, n);
	LogicBlock& one = sub.add(LogicBlockOn);
	LogicBlock& tempAdd = createNBitAdd(board, sub, n);
	LogicBlock& add = createNBitAdd(board, sub, n);
	LogicBlock& ret = sub.add(LogicBlockNot);

	board.connectToIn(sub, "b[]", inv, "in[]"); // 0
	board.connect(inv, "out[]", tempAdd, "a[]"); // 1
	board.connect(one, "out", tempAdd, "b[0]"); // 0

	board.connectToIn(sub, "a[]", add, "a[]"); // 0
	board.connect(tempAdd, "out[]", add, "b[]"); // 2

	board.connectToOut(add, "out[]", sub, "out[]"); // 3
	board.connect(add, "overflow", ret, "in"); // 3
	board.connectToOut(ret, "out", sub, "overflow"); // 4

	return sub;
}

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

static LogicBlock& createNBitCompare(Board& board, LogicBlock& parent, int n)
{
	LogicBlock& comp = parent.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n), 
		"out", std::to_string(n) + "bitCompare"));

	LogicBlock& nxor = createNBitXor(board, comp, n);
	LogicBlock& iszero = createNBitIsZero(board, comp, n);

	board.connectToIn(comp, "a[]", nxor, "a[]");
	board.connectToIn(comp, "b[]", nxor, "b[]");

	board.connect(nxor, "out[]", iszero, "in[]");
	board.connectToOut(iszero, "out", comp, "out");

	return comp;
}


int main()
{
	Board board;

	int n = 32;
	LogicBlock& a = createNumberNBit(board, board.board, n, 54321);
	LogicBlock& b = createNumberNBit(board, board.board, n, 12345);
	LogicBlock& sub = createNBitSub(board, board.board, n);
	
	board.connect(a, "out[]", sub, "a[]");
	board.connect(b, "out[]", sub, "b[]");
	
	for (int ii = 0; ii < n * 3; ++ii)
		for (int i = 0; i < 3; ++i)
			board.tick();

	sub.print();

	//LogicBlock& compare = createNBitCompare(board, 64);
	//LogicBlock& num0 = createNumberNBit(board, 45, 64);
	//LogicBlock& num1 = createNumberNBit(board, 45, 64);
	//board.connect(num0, "out[]", compare, "a[]");
	//board.connect(num1, "out[]", compare, "b[]");
	//for (int i = 0; i < 10; ++i)
	//	board.tick();
	//compare.print();

	return 0;
}