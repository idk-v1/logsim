#include "logicblock.h"

LogicBlock& createFullAdd(Board& board)
{
	LogicBlock& fullAdd = board.add(LogicBlock("a|b|c", "out|carry", "fullAdd"));

	LogicBlock& tempxor0   = board.add(LogicBlock("xor"));
	LogicBlock& tempand0   = board.add(LogicBlock("and"));
	LogicBlock& tempxor1   = board.add(LogicBlock("xor"));
	LogicBlock& tempand1   = board.add(LogicBlock("and"));
	LogicBlock& tempor     = board.add(LogicBlock("or"));
	LogicBlock& tempdelay0 = board.add(LogicBlock("delay"));
	LogicBlock& tempdelay1 = board.add(LogicBlock("delay"));
	LogicBlock& tempdelay2 = board.add(LogicBlock("delay"));

	board.connectToIn(fullAdd, "a", tempxor0, "a");
	board.connectToIn(fullAdd, "b", tempxor0, "b");

	board.connectToIn(fullAdd, "a", tempand0, "a");
	board.connectToIn(fullAdd, "b", tempand0, "b");
	board.connect(tempand0,    "out", tempdelay0, "in");

	board.connectToIn(fullAdd, "c", tempdelay2, "in");

	board.connect(tempxor0,        "out", tempxor1,   "a");
	board.connect(tempdelay2,      "out", tempxor1,   "b");
	board.connect(tempxor1,        "out", tempdelay1, "in");
	board.connectToOut(tempdelay1, "out", fullAdd,    "out");

	board.connect(tempxor0,   "out", tempand1, "a");
	board.connect(tempdelay2, "out", tempand1, "b");
	board.connect(tempand1,    "out", tempor,  "a");
	board.connect(tempdelay0,  "out", tempor,  "b");
	board.connectToOut(tempor, "out", fullAdd, "carry");

	return fullAdd;
}

LogicBlock& createFullAddFast(Board& board)
{
	LogicBlock& fullAdd = board.add(LogicBlock("a|b|c0|c1", "out|carry0|carry1", "fullAdd"));

	LogicBlock& tempxor0 = board.add(LogicBlock("xor"));
	LogicBlock& tempand0 = board.add(LogicBlock("and"));
	LogicBlock& tempxor1 = board.add(LogicBlock("xor"));
	LogicBlock& tempand1 = board.add(LogicBlock("and"));
	LogicBlock& tempor = board.add(LogicBlock("or"));
	LogicBlock& tempdelay0 = board.add(LogicBlock("delay"));

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

LogicBlock& createNumberNBit(Board& board, int64_t input, int n)
{
	LogicBlock& num = board.add(LogicBlock("", arrayToStr("out", n), std::to_string(n) + "bitNumber"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& temp = board.add(LogicBlock((input & 1) ? "on" : "off"));
		board.connectToOut(temp, "out", num, arrayToStr("out", i));
		input >>= 1;
	}

	return num;
}

LogicBlock& createNBitAdd(Board& board, int n)
{
	LogicBlock& add = board.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n), 
		arrayToStr("out", n) + "|carry", std::to_string(n) + "bitAdder"));

	LogicBlock* oldAdd = NULL;

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempAdd = createFullAddFast(board);
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
		LogicBlock& tempor = board.add(LogicBlock("or"));
		board.connect(*oldAdd, "carry0", tempor, "a");
		board.connect(*oldAdd, "carry1", tempor, "b");
		board.connectToOut(tempor, "out", add, "carry");
	}

	return add;
}

LogicBlock& createNBitInvert(Board& board, int n)
{
	LogicBlock& inv = board.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n), std::to_string(n) + "bitInvert"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempNot = board.add(LogicBlock("not"));
		board.connectToIn(inv, arrayToStr("in", i), tempNot, "in");
		board.connectToOut(tempNot, "out", inv, arrayToStr("out", i));
	}

	return inv;
}

LogicBlock& createNBitSub(Board& board, int n)
{
	LogicBlock& sub = board.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n),
		arrayToStr("out", n) + "|carry", std::to_string(n) + "bitSubtract"));

	LogicBlock& inv = createNBitInvert(board, n);
	LogicBlock& one = board.add(LogicBlock("on"));
	LogicBlock& tempAdd = createNBitAdd(board, n);
	LogicBlock& add = createNBitAdd(board, n);
	LogicBlock& ret = board.add(LogicBlock("not"));

	board.connectToIn(sub, "b[]", inv, "in[]");
	board.connect(inv, "out[]", tempAdd, "a[]");
	board.connect(one, "out", tempAdd, "b[0]");

	board.connectToIn(sub, "a[]", add, "a[]");
	board.connect(tempAdd, "out[]", add, "b[]");

	board.connectToOut(add, "out[]", sub, "out[]");
	board.connect(add, "carry", ret, "in");
	board.connectToOut(ret, "out", sub, "carry");

	return sub;
}

LogicBlock& createNBitXor(Board& board, int n)
{
	LogicBlock& nxor = board.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n), std::to_string(n) + "bitXor"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempXor = board.add(LogicBlock("xor"));
		board.connectToIn(nxor, arrayToStr("in", i), tempXor, "in");
		board.connectToOut(tempXor, "out", nxor, arrayToStr("out", i));
	}

	return nxor;
}

LogicBlock& createNBitIsZero(Board& board, int n)
{
	LogicBlock& iszero = board.add(LogicBlock(arrayToStr("in", n), "out", std::to_string(n) + "bitIsZero"));

	std::vector<LogicBlock*> ors;

	for (int i = 0; i < n; ++i)
	{
		if (i % 2 == 0)
		{
			LogicBlock& tempor = board.add(LogicBlock("or"));
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
				LogicBlock& tempor = board.add(LogicBlock("or"));
				ors.push_back(&tempor);
				board.connect(*(ors[start + i]), "out", *ors.back(), "a");
			}
			else
				board.connect(*(ors[start + i]), "out", *ors.back(), "b");

		}
		len = (len + 1) / 2;
		start = ors.size() - len;
	}

	LogicBlock& tempnot = board.add(LogicBlock("not"));
	board.connect(*(ors.back()), "out", tempnot, "in");
	board.connectToOut(tempnot, "out", iszero, "out");

	return iszero;
}

LogicBlock& createNBitCompare(Board& board, int n)
{
	LogicBlock& comp = board.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n), "out", std::to_string(n) + "bitCompare"));

	LogicBlock& nxor = createNBitXor(board, n);
	LogicBlock& iszero = createNBitIsZero(board, n);

	board.connectToIn(comp, "a[]", nxor, "a[]");
	board.connectToIn(comp, "b[]", nxor, "b[]");

	board.connect(nxor, "out[]", iszero, "in[]");
	board.connectToOut(iszero, "out", comp, "out");

	return comp;
}

int main()
{
	Board board;

	//LogicBlock& a = createNumberNBit(board, 54321, 64);
	//LogicBlock& sub = createNBitSub(board, 64);
	//LogicBlock& b = createNumberNBit(board, 12345, 64);
	//
	//board.connect(a, "out[]", sub, "a[]");
	//board.connect(b, "out[]", sub, "b[]");
	//
	//for (int ii = 0; ii < 64 * 3; ++ii)
	//{
	//	for (int i = 0; i < 3; ++i)
	//	{
	//		board.tick();
	//	}
	//}
	//sub.print();

	LogicBlock& compare = createNBitCompare(board, 64);
	LogicBlock& num0 = createNumberNBit(board, 45, 64);
	LogicBlock& num1 = createNumberNBit(board, 45, 64);
	board.connect(num0, "out[]", compare, "a[]");
	board.connect(num1, "out[]", compare, "b[]");
	for (int i = 0; i < 10; ++i)
		board.tick();
	compare.print();

	return 0;
}