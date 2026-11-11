#pragma once

#include "logicsim.h"
#include "../graphics/graphics.h"
#include "../graphics/bulletinV1_font.h"

// out: out[]
static LogicBlock& createNBitNumber(LogicSim& sim, LogicBlock& parent, int n, int64_t input)
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

// in: in[]
static LogicBlock& createNBitOutput(LogicSim& sim, LogicBlock& parent, int n)
{
	return parent.add(LogicBlock(arrayToStr("in", n), "", std::to_string(n) + "bitOutput"));
}


// n cycles
// in: in
// out: out
static LogicBlock& createNDelay(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& delay = parent.add(LogicBlock("in", "out", std::to_string(n) + "delay"));

	LogicBlock* old = NULL;
	for (int i = 0; i < n; ++i)
	{
		LogicBlock& temp = delay.add(LogicBlockDelay);
		temp.setPos(15 + (i / 10) * 50, 15 + (i % 10) * 30);
		if (i == 0)
			sim.connect(delay, "in", temp, "in", inin);
		else if (old)
			sim.connect(*old, "out", temp, "in");

		old = &temp;
	}
	if (old)
		sim.connect(*old, "out", delay, "out", outout);
	else
		sim.connect(delay, "in", delay, "out", inout);

	return delay;
}

// n cycles
// in: in[]
// out: out[]
static LogicBlock& createNDelayBuf(LogicSim& sim, LogicBlock& parent, int delay, int n)
{
	LogicBlock& buf = parent.add(LogicBlock(arrayToStr("in", n), 
		arrayToStr("out", n), std::to_string(n) + "x" + 
		std::to_string(delay) + "delay"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& temp = createNDelay(sim, buf, delay);
		sim.connect(buf, arrayToStr("in", i), temp, "in", inin);
		sim.connect(temp, "out", buf, arrayToStr("out", i), outout);
	}

	return buf;
}

// 1 cycle
// in: a, b
// out: out, carry
static LogicBlock& createHalfAdd(LogicSim& sim, LogicBlock& parent)
{
	LogicBlock& add = parent.add(LogicBlock("a|b", "out|carry", "halfAdd"));

	LogicBlock& tempxor = add.add(LogicBlockXor);
	LogicBlock& tempand = add.add(LogicBlockAnd);

	sim.connect(add, "a", tempxor, "a", inin);
	sim.connect(add, "b", tempxor, "b", inin);
	sim.connect(add, "a", tempand, "a", inin);
	sim.connect(add, "b", tempand, "b", inin);

	sim.connect(tempxor, "out", add, "out", outout);
	sim.connect(tempand, "out", add, "carry", outout);

	return add;
}

// n cycles
// in: a[], sigin
// out: out[], overflow, sigout
static LogicBlock& createNBitAdd1(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& add = parent.add(LogicBlock(arrayToStr("a", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitAdd1"));

	LogicBlock* oldAdd = NULL;
	LogicBlock& one = add.add(LogicBlockOn);

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempAdd = createHalfAdd(sim, add);
		sim.connect(add, arrayToStr("a", i), tempAdd, "a", inin);

		if (i > 0)
			sim.connect(*oldAdd, "carry", tempAdd, "b");
		else
			sim.connect(one, "out", tempAdd, "b");

		sim.connect(tempAdd, "out", add, arrayToStr("out", i), outout);

		oldAdd = &tempAdd;
	}
	if (oldAdd)
		sim.connect(*oldAdd, "carry", add, "overflow", outout);

	LogicBlock& delay = createNDelay(sim, add, n);
	delay.setExpand(false);
	sim.connect(add, "sigin", delay, "in", inin);
	sim.connect(delay, "out", add, "sigout", outout);

	return add;
}

// 2 cycles
// in: a, b, c
// out: out, carry
static LogicBlock& createFullAdd(LogicSim& sim, LogicBlock& parent)
{
	LogicBlock& fullAdd = parent.add(LogicBlock("a|b|c", "out|carry", "fullAdd"));

	LogicBlock& tempxor0 = fullAdd.add(LogicBlockXor);
	LogicBlock& tempand0 = fullAdd.add(LogicBlockAnd);
	LogicBlock& tempxor1 = fullAdd.add(LogicBlockXor);
	LogicBlock& tempand1 = fullAdd.add(LogicBlockAnd);
	LogicBlock& tempdelay0 = fullAdd.add(LogicBlockDelay);
	LogicBlock& tempdelay1 = fullAdd.add(LogicBlockDelay);

	// step 0
	// tempxor0
	// tempand0
	// tempdelay1

	// step 1
	// tempdelay0
	// tempxor1
	// tempand1

	sim.connect(fullAdd, "a", tempxor0, "a", inin); // 0
	sim.connect(fullAdd, "b", tempxor0, "b", inin); // 0

	sim.connect(fullAdd, "a", tempand0, "a", inin); // 0
	sim.connect(fullAdd, "b", tempand0, "b", inin); // 0
	sim.connect(tempand0, "out", tempdelay0, "in"); // 1

	sim.connect(fullAdd, "c", tempdelay1, "in", inin); // 0

	sim.connect(tempxor0, "out", tempxor1, "a"); // 1
	sim.connect(tempdelay1, "out", tempxor1, "b"); // 1
	sim.connect(tempxor1, "out", fullAdd, "out", outout); // 2

	sim.connect(tempxor0, "out", tempand1, "a"); // 1
	sim.connect(tempdelay1, "out", tempand1, "b"); // 1
	sim.connect(tempand1, "out", fullAdd, "carry", outout); // 2
	sim.connect(tempdelay0, "out", fullAdd, "carry", outout); // 2

	return fullAdd;
}

// 2n cycles
// in: a[], b[], sigin
// out: out[], overflow, sigout
static LogicBlock& createNBitAdd(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& add = parent.add(LogicBlock(
		arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitAdder"));

	LogicBlock* oldAdd = NULL;

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempAdd = createFullAdd(sim, add);
		tempAdd.setPos(15 + (i / (n / 4)) * 140, 15 + (i % (n / 4)) * 105);
		tempAdd.setExpand(false);
		sim.connect(add, arrayToStr("a", i), tempAdd, "a", inin);
		sim.connect(add, arrayToStr("b", i), tempAdd, "b", inin);

		if (i > 0)
		{
			sim.connect(*oldAdd, "carry", tempAdd, "c");
		}

		sim.connect(tempAdd, "out", add, arrayToStr("out", i), outout);

		oldAdd = &tempAdd;
	}

	if (oldAdd) // no one would make a 0bit adder
		sim.connect(*oldAdd, "carry", add, "overflow", outout);

	LogicBlock& delay = createNDelay(sim, add, 2 * n);
	delay.setPos(15 + ((n + 1) / (n / 4)) * 140, 15 + ((n + 1) % (n / 4)) * 50);
	delay.setExpand(false);
	sim.connect(add, "sigin", delay, "in", inin);
	sim.connect(delay, "out", add, "sigout", outout);

	return add;
}


// 1 cycle
// in: in[]
// out: out[]
static LogicBlock& createNBitInvert(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& inv = parent.add(LogicBlock(arrayToStr("in", n),
		arrayToStr("out", n), std::to_string(n) + "bitInvert"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempNot = inv.add(LogicBlockNot);
		sim.connect(inv, arrayToStr("in", i), tempNot, "in", inin);
		sim.connect(tempNot, "out", inv, arrayToStr("out", i), outout);
	}

	return inv;
}


// 2n cycles
// in: a[], sigin
// out: out[], sigout
static LogicBlock& createNBitSub1(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& sub = parent.add(LogicBlock(arrayToStr("a", n) + "|sigin", 
		arrayToStr("out", n) + "|sigout", std::to_string(n) + "bitSub1"));

	LogicBlock& add = createNBitAdd(sim, sub, n);
	LogicBlock& num = createNBitInvert(sim, sub, n);

	sim.connect(sub, "a[]", add, "a[]", inin);
	sim.connect(sub, "sigin", add, "sigin", inin);
	sim.connect(num, "out[]", add, "b[]");
	sim.connect(add, "out[]", sub, "out[]", outout);
	sim.connect(add, "sigout", sub, "sigout", outout);

	return sub;
}


// n + 1 cycles
// in: in[], sigin
// out: out[], sigout
static LogicBlock& createNBitNegate(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& neg = parent.add(LogicBlock(arrayToStr("in", n) + "|sigin",
		arrayToStr("out", n) + "|sigout", std::to_string(n) + "bitNegate"));

	LogicBlock& inv = createNBitInvert(sim, neg, n);
	LogicBlock& add = createNBitAdd1(sim, neg, n);
	LogicBlock& delay = neg.add(LogicBlockDelay);

	sim.connect(neg, "in[]", inv, "in[]", inin);
	sim.connect(neg, "sigin", delay, "in", inin);
	sim.connect(inv, "out[]", add, "a[]");
	sim.connect(delay, "out", add, "sigin");
	sim.connect(add, "out[]", neg, "out[]", outout);
	sim.connect(add, "sigout", neg, "sigout", outout);

	return neg;
}

// 3n + 3 cycles
// in: a[], b[], sigin
// out: out[], overflow, sigout
static LogicBlock& createNBitSub(LogicSim& sim, LogicBlock& parent, int n)
{
	// overflow (integer underflow) needs the high bit of a to change and there to be an overflow
	LogicBlock& sub = parent.add(LogicBlock(
		arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitSubtract"));

	LogicBlock& neg = createNBitNegate(sim, sub, n);
	LogicBlock& add = createNBitAdd(sim, sub, n);
	LogicBlock& highxor = sub.add(LogicBlockXor);
	LogicBlock& retand = sub.add(LogicBlockAnd);
	LogicBlock& delay = createNDelay(sim, sub, 2);
	neg.setExpand(false);
	neg.setPos(15, 10);
	add.setExpand(false);
	add.setPos(300, 15);
	retand.setPos(450, 100);
	highxor.setPos(450, 70);
	delay.setExpand(false);

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

	sim.connect(sub, "sigin", neg, "sigin", inin); // 0
	sim.connect(neg, "sigout", add, "sigin"); // 1

	sim.connect(sub, "b[]", neg, "in[]", inin); // 0

	sim.connect(sub, "a[]", add, "a[]", inin); // 0
	sim.connect(neg, "out[]", add, "b[]"); // 1

	sim.connect(add, "out[]", sub, "out[]", outout); // 2
	sim.connect(add, arrayToStr("out", n - 1), highxor, "a"); // 2
	sim.connect(sub, arrayToStr("a", n - 1), highxor, "b", inin); // 0
	sim.connect(add, "overflow", retand, "a"); // 3
	sim.connect(highxor, "out", retand, "b"); // 3
	sim.connect(retand, "out", sub, "overflow", outout); // 4

	sim.connect(add, "sigout", delay, "in"); // 2
	sim.connect(delay, "out", sub, "sigout", outout); // 4

	return sub;
}

// 1 cycle
// in: a[], b[]
// out: out[]
static LogicBlock& createNBitXor(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& nxor = parent.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n),
		arrayToStr("out", n), std::to_string(n) + "bitXor"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempXor = nxor.add(LogicBlockXor);
		sim.connect(nxor, arrayToStr("a", i), tempXor, "a", inin);
		sim.connect(nxor, arrayToStr("b", i), tempXor, "b", inin);
		sim.connect(tempXor, "out", nxor, arrayToStr("out", i), outout);
	}

	return nxor;
}

// 1 cycle
// in: in[]
// out: out
static LogicBlock& createNBitIsZero(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& iszero = parent.add(LogicBlock(arrayToStr("in", n), "out",
		std::to_string(n) + "bitIsZero"));

	LogicBlock& tempnot = iszero.add(LogicBlockNot);
	sim.connect(tempnot, "out", iszero, "out", outout);

	for (int i = 0; i < n; ++i)
		sim.connect(iszero, arrayToStr("in", i), tempnot, "in", inin);
	return iszero;
}

// 2 cycles
// in: a[], b[]
// out: out
static LogicBlock& createNBitIsEqual(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& comp = parent.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n),
		"out", std::to_string(n) + "bitIsEqual"));

	LogicBlock& nxor = createNBitXor(sim, comp, n);
	LogicBlock& iszero = createNBitIsZero(sim, comp, n);

	// step 0
	// nxor

	// step 1
	// iszero

	sim.connect(comp, "a[]", nxor, "a[]", inin);
	sim.connect(comp, "b[]", nxor, "b[]", inin);

	sim.connect(nxor, "out[]", iszero, "in[]");
	sim.connect(iszero, "out", comp, "out", outout);

	return comp;
}

// 1 cycle
// in: in[], write
// out: out[]
static LogicBlock& createNBitGate(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& gate = parent.add(LogicBlock(arrayToStr("in", n) + "|write",
		arrayToStr("out", n), std::to_string(n) + "bitGate"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempand = gate.add(LogicBlockAnd);
		tempand.setPos(5, i * 30 + 5);
		sim.connect(gate, arrayToStr("in", i), tempand, "a", inin);
		sim.connect(gate, "write", tempand, "b", inin);
		sim.connect(tempand, "out", gate, arrayToStr("out", i), outout);
	}

	return gate;
}

// 2 cycles
// in: in[]
// out: out
static LogicBlock& createNBitIsEqualConst(LogicSim& sim, LogicBlock& parent, int n, int64_t value)
{
	LogicBlock& equal = parent.add(LogicBlock(arrayToStr("in", n), 
		"out", std::to_string(n) + "bitIsEqualConst"));

	LogicBlock& num = createNBitNumber(sim, equal, n, value);
	LogicBlock& isequal = createNBitIsEqual(sim, equal, n);
	sim.connect(equal, "in[]", isequal, "a[]", inin);
	sim.connect(num, "out[]", isequal, "b[]");
	sim.connect(isequal, "out", equal, "out", outout);

	return equal;
}

// 3 cycles
// in: num[], data[], sigin
// out: data[], sigout
static LogicBlock& createNBitIsEqualConstDataSigGate(LogicSim& sim, LogicBlock& parent, int n, int64_t value)
{
	LogicBlock& ecdsg = parent.add(LogicBlock(arrayToStr("num", n) + arrayToStr("data", n) + "|sigin",
		arrayToStr("data[]", n) + "|sigout", std::to_string(n) + "bitIsEqualConstDataSigGate"));

	LogicBlock& equal = createNBitIsEqualConst(sim, ecdsg, n, value);
	LogicBlock& datagate = createNBitGate(sim, ecdsg, n);
	LogicBlock& siggate = ecdsg.add(LogicBlockAnd);
	LogicBlock& delay = createNDelay(sim, ecdsg, 2);

	sim.connect(ecdsg, "num[]", equal, "in[]", inin);
	sim.connect(ecdsg, "data[]", datagate, "in[]", inin);
	sim.connect(ecdsg, "sigin", delay, "in", inin);
	sim.connect(equal, "out", datagate, "write");
	sim.connect(delay, "out", siggate, "a");
	sim.connect(equal, "out", siggate, "b");
	sim.connect(datagate, "out[]", ecdsg, "data[]", outout);
	sim.connect(siggate, "out", ecdsg, "sigout", outout);

	return ecdsg;
}


// 1 cycle
// in: in[]
// out: out[]
static LogicBlock& createNBitShiftLeftM(LogicSim& sim, LogicBlock& parent, int n, int m)
{
	LogicBlock& shift = parent.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n),
		std::to_string(n) + "bitShiftLeft" + std::to_string(m)));

	for (int i = 0; i < n; ++i)
	{
		int index = i + m;
		if (index >= 0 && index < n)
			sim.connect(shift, arrayToStr("in", i), shift, arrayToStr("out", index), inout);
	}

	return shift;
}

// 1 cycle
// in: in[]
// out: out[]
static LogicBlock& createNBitShiftRightM(LogicSim& sim, LogicBlock& parent, int n, int m)
{
	LogicBlock& shift = parent.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n),
		std::to_string(n) + "bitShiftRight" + std::to_string(m)));

	for (int i = 0; i < n; ++i)
	{
		int index = i - m;
		if (index >= 0 && index < n)
			sim.connect(shift, arrayToStr("in", i), shift, arrayToStr("out", index), inout);
	}

	return shift;
}

// 1 cycle
// in: in[]
// out: out[]
static LogicBlock& createNBitShiftRightArithM(LogicSim& sim, LogicBlock& parent, int n, int m)
{
	LogicBlock& shift = parent.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n),
		std::to_string(n) + "bitShiftRightArith" + std::to_string(m)));

	for (int i = 0; i < n; ++i)
	{
		int index = i - m;
		if (index >= 0 && index < n)
			sim.connect(shift, arrayToStr("in", i), shift, arrayToStr("out", index), inout);
	}
	for (int i = 1; i <= m; ++i)
		sim.connect(shift, arrayToStr("in", n - 1), shift, arrayToStr("out", n - m), inout);

	return shift;
}

// 1 cycle
// in: in[]
// out: out[]
static LogicBlock& createNBitCycleLeftM(LogicSim& sim, LogicBlock& parent, int n, int m)
{
	LogicBlock& shift = parent.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n),
		std::to_string(n) + "bitCycleLeft" + std::to_string(m)));

	for (int i = 0; i < n; ++i)
	{
		int index = (i + m) % n;
		sim.connect(shift, arrayToStr("in", i), shift, arrayToStr("out", index), inout);
	}

	return shift;
}

// 1 cycle
// in: in[]
// out: out[]
static LogicBlock& createNBitCycleRightM(LogicSim& sim, LogicBlock& parent, int n, int m)
{
	LogicBlock& shift = parent.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n),
		std::to_string(n) + "bitCycleRight" + std::to_string(m)));

	for (int i = 0; i < n; ++i)
	{
		int index = (i - m + n) % n;
		sim.connect(shift, arrayToStr("in", i), shift, arrayToStr("out", index), inout);
	}

	return shift;
}


// 3 cycles
// in: in[]
// out: out[]
static LogicBlock& createNBitHighestBit(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& high = parent.add(LogicBlock(
		arrayToStr("in", n), arrayToStr("out", n), std::to_string(n) + "bitHighestBit"));

	std::vector<LogicBlock*> nots;
	for (int i = 0; i < n; ++i)
	{
		LogicBlock& temp0 = high.add(LogicBlockNot);
		LogicBlock& temp1 = high.add(LogicBlockNot);
		temp0.setPos(15, 15 + i * 40);
		temp1.setPos(75, 15 + i * 40);
		sim.connect(high, arrayToStr("in", i), temp0, "in", inin);
		sim.connect(temp0, "out", temp1, "in");
		sim.connect(temp1, "out", high, arrayToStr("out", i), outout);
		nots.push_back(&temp0);
		nots.push_back(&temp1);
	}

	for (int i = n - 1; i >= 0; --i)
		for (int ii = i - 1; ii >= 0; --ii)
			sim.connect(*nots[i * 2 + 1], "out", *nots[ii * 2 + 1], "in");

	return high;
}

// 3 cycles
// in: in[]
// out: out[]
static LogicBlock& createNBitLowestBit(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& high = parent.add(LogicBlock(
		arrayToStr("in", n), arrayToStr("out", n), std::to_string(n) + "bitLowestBit"));

	std::vector<LogicBlock*> nots;
	for (int i = 0; i < n; ++i)
	{
		LogicBlock& temp0 = high.add(LogicBlockNot);
		LogicBlock& temp1 = high.add(LogicBlockNot);
		temp0.setPos(15, 15 + i * 40);
		temp1.setPos(75, 15 + i * 40);
		sim.connect(high, arrayToStr("in", i), temp0, "in", inin);
		sim.connect(temp0, "out", temp1, "in");
		sim.connect(temp1, "out", high, arrayToStr("out", i), outout);
		nots.push_back(&temp0);
		nots.push_back(&temp1);
	}

	for (int i = 0; i < n; ++i)
		for (int ii = i + 1; ii < n; ++ii)
			sim.connect(*nots[i * 2 + 1], "out", *nots[ii * 2 + 1], "in");

	return high;
}


// 2 cycles
// in: a[], b[]
// out: a[], b[]
static LogicBlock& createNBitDiff(LogicSim& sim, LogicBlock& parent, int n)
{
	// returns the missing bits of each input
	// a = 0b01010111 (+087)
	// b = 0b00110011 (+051)
	// kindof an xor, but it tells you what side the on came from
	// a, b -> not, pass -> and -> 0b00100000 a
	// a, b -> pass, not -> and -> 0b01000100 b

	LogicBlock& diff = parent.add(LogicBlock(
		arrayToStr("a", n) + "|" + arrayToStr("b", n), 
		arrayToStr("a", n) + "|" + arrayToStr("b", n),
		std::to_string(n) + "bitDiff"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& passA = diff.add(LogicBlockDelay);
		LogicBlock& passB = diff.add(LogicBlockDelay);
		LogicBlock& notA = diff.add(LogicBlockNot);
		LogicBlock& notB = diff.add(LogicBlockNot);
		LogicBlock& andA = diff.add(LogicBlockAnd);
		LogicBlock& andB = diff.add(LogicBlockAnd);

		sim.connect(diff, arrayToStr("a", i), notA, "in", inin);
		sim.connect(diff, arrayToStr("b", i), passA, "in", inin);
		sim.connect(notA, "out", andA, "a");
		sim.connect(passA, "out", andA, "b");
		sim.connect(andA, "out", diff, arrayToStr("a", i), outout);

		sim.connect(diff, arrayToStr("a", i), passB, "in", inin);
		sim.connect(diff, arrayToStr("b", i), notB, "in", inin);
		sim.connect(passB, "out", andB, "a");
		sim.connect(notB, "out", andB, "b");
		sim.connect(andB, "out", diff, arrayToStr("b", i), outout);
	}

	return diff;
}

// 8 cycles
// in: a[], b[], sigin
// out: less, equal, greater, sigout
static LogicBlock& createNBitCompareUns(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& less = parent.add(LogicBlock(
		arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		"less|equal|greater|sigout", std::to_string(n) + "bitCompareUns"));

	LogicBlock& sigdelay = createNDelay(sim, less, 8);
	sim.connect(less, "sigin", sigdelay, "in", inin);
	sim.connect(sigdelay, "out", less, "sigout", outout);

	LogicBlock& diff = createNBitDiff(sim, less, n);
	LogicBlock& iszeroA = createNBitIsZero(sim, less, n);
	LogicBlock& passA = less.add(LogicBlockDelay);
	LogicBlock& notA = less.add(LogicBlockNot);
	LogicBlock& gateA = createNBitGate(sim, less, 2);
	LogicBlock& isequal = createNBitIsEqual(sim, less, n);
	LogicBlock& isequalinv = less.add(LogicBlockNot);

	sim.connect(less, "a[]", diff, "a[]", inin);
	sim.connect(less, "b[]", diff, "b[]", inin);

	// cancel less/greater if equal
	sim.connect(less, "a[]", isequal, "a[]", inin);
	sim.connect(less, "b[]", isequal, "b[]", inin);
	sim.connect(isequal, "out", isequalinv, "in");
	sim.connect(isequalinv, "out", gateA, "write");
	sim.connect(gateA, "out[0]", less, "less", outout);
	sim.connect(gateA, "out[1]", less, "greater", outout);
	sim.connect(isequal, "out", less, "equal", outout);

	std::vector<LogicBlock*> notsA;
	std::vector<LogicBlock*> notsB;
	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempA0 = less.add(LogicBlockNot);
		LogicBlock& tempA1 = less.add(LogicBlockNot);
		tempA0.setPos(15, 15 + i * 40);
		tempA1.setPos(75, 15 + i * 40);
		sim.connect(diff, arrayToStr("a", i), tempA0, "in");
		sim.connect(tempA0, "out", tempA1, "in");
		sim.connect(tempA1, "out", iszeroA, arrayToStr("in", i));
		notsA.push_back(&tempA0);
		notsA.push_back(&tempA1);

		LogicBlock& tempB0 = less.add(LogicBlockNot);
		LogicBlock& tempB1 = less.add(LogicBlockNot);
		tempB0.setPos(215, 15 + i * 40);
		tempB1.setPos(275, 15 + i * 40);
		sim.connect(diff, arrayToStr("b", i), tempB0, "in");
		sim.connect(tempB0, "out", tempB1, "in");
		notsB.push_back(&tempB0);
		notsB.push_back(&tempB1);
	}

	for (int i = n - 1; i >= 0; --i)
		for (int ii = i - 1; ii >= 0; --ii)
		{
			sim.connect(*notsA[i * 2 + 1], "out", *notsA[ii * 2 + 1], "in");
			sim.connect(*notsA[i * 2 + 1], "out", *notsB[ii * 2 + 1], "in");
			sim.connect(*notsB[i * 2 + 1], "out", *notsA[ii * 2 + 1], "in");
			sim.connect(*notsB[i * 2 + 1], "out", *notsB[ii * 2 + 1], "in");
		}

	// if a is zero, b is non zero, so b < a
	sim.connect(iszeroA, "out", notA, "in");
	sim.connect(notA, "out", gateA, "in[0]");
	sim.connect(iszeroA, "out", passA, "in");
	sim.connect(passA, "out", gateA, "in[1]");

	/*
	find highest bit could be modified to use the directional xor from above

	  a
	3 0
	2 1
	1 1
	0 0
	
	  b
	3 0
	2 1
	1 0
	0 1
	
	bitDiff(1110, 0101) -> (0001, 1010)
	
	a -> not -> not -> a
	0 ->   1 ->   0 -> 0
	0 ->   1 ->   0 -> 0
	0 ->   1 ->   0 -> 0
	1 ->   0 ->   1 -> 0
	
	
	b -> not -> not -> b
	1 ->   0 ->   1 -> 1
	0 ->   1 ->   0 -> 0
	1 ->   0 ->   1 -> 0
	0 ->   1 ->   0 -> 0
	
	basically gethighestbit, but the cancelling is also wired to the other output
	
	b is non-zero, so it is the lowest, false for less, true for greater
	if a is non-zero, it is lowest, true for less, false for greater
	need a way to block output if is equal
	
	above takes 5 cycles
	+ iszero + (not,pass) + gate for output
	isequal takes 2 cycles
	
	total = 5 + 1 + 1 + 1 = 8
	*/

	return less;
}

// 10(pp) or 3(pn) or 11(nn) cycles
// in: a[], b[], sigin
// out: less, equal, greater, sigout
static LogicBlock& createNBitCompare(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& less = parent.add(LogicBlock(
		arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		"less|equal|greater|sigout", std::to_string(n) + "bitCompare"));

	LogicBlock& lessUns = createNBitCompareUns(sim, less, n);
	lessUns.setExpand(false);
	lessUns.setPos(200, 0);

	LogicBlock& delay = createNDelay(sim, less, 2);
	delay.setExpand(false);
	sim.connect(less, "sigin", delay, "in", inin);

	LogicBlock& equal = createNBitIsEqual(sim, less, n);
	sim.connect(less, "a[]", equal, "a[]", inin);
	sim.connect(less, "b[]", equal, "b[]", inin);
	sim.connect(equal, "out", less, "equal", outout);

	// Positive positive
	// positive positive and negative negative are going to be much slower than mixed signs,
	// so a 1 cycles speed up is good
	LogicBlock& andPP = less.add(LogicBlockNor);
	LogicBlock& gatePPa = createNBitGate(sim, less, n);
	gatePPa.setExpand(false);
	LogicBlock& gatePPb = createNBitGate(sim, less, n);
	gatePPb.setExpand(false);
	LogicBlock& gatePPs = less.add(LogicBlockAnd);
	andPP.setPos(15, 105);
	gatePPs.setPos(15, 150);
	gatePPa.setPos(100, 15);
	gatePPb.setPos(100, 100);
	sim.connect(less, arrayToStr("a", n - 1), andPP, "a", inin);
	sim.connect(less, arrayToStr("b", n - 1), andPP, "b", inin);
	sim.connect(andPP, "out", gatePPa, "write");
	sim.connect(andPP, "out", gatePPb, "write");
	sim.connect(less, "a[]", gatePPa, "in[]", inin);
	sim.connect(less, "b[]", gatePPb, "in[]", inin);
	sim.connect(gatePPa, "out[]", lessUns, "a[]");
	sim.connect(gatePPb, "out[]", lessUns, "b[]");
	sim.connect(delay, "out", gatePPs, "a");
	sim.connect(andPP, "out", gatePPs, "b");
	sim.connect(gatePPs, "out", lessUns, "sigin");
	sim.connect(lessUns, "less", less, "less", outout);
	sim.connect(lessUns, "greater", less, "greater", outout);
	sim.connect(lessUns, "sigout", less, "sigout", outout);

	// Positive negative
	LogicBlock& notPNa = less.add(LogicBlockNot);
	LogicBlock& passPNb = less.add(LogicBlockDelay);
	LogicBlock& andPN = less.add(LogicBlockAnd);
	LogicBlock& gatePNs = less.add(LogicBlockAnd);
	notPNa.setPos(15, 15 + 185);
	passPNb.setPos(15, 60 + 185);
	andPN.setPos(15, 105 + 185);
	gatePNs.setPos(15, 150 + 185);
	sim.connect(less, arrayToStr("a", n - 1), notPNa, "in", inin);
	sim.connect(less, arrayToStr("b", n - 1), passPNb, "in", inin);
	sim.connect(notPNa, "out", andPN, "a");
	sim.connect(passPNb, "out", andPN, "b");
	sim.connect(delay, "out", gatePNs, "a");
	sim.connect(andPN, "out", gatePNs, "b");
	// signal is always gonna be on, this path always returns true for greater
	sim.connect(gatePNs, "out", less, "greater", outout);
	// no need to return false for less
	sim.connect(gatePNs, "out", less, "sigout", outout);

	// Negative positive
	LogicBlock& passNPa = less.add(LogicBlockDelay);
	LogicBlock& notNPb = less.add(LogicBlockNot);
	LogicBlock& andNP = less.add(LogicBlockAnd);
	LogicBlock& gateNPs = less.add(LogicBlockAnd);
	passNPa.setPos(15, 15 + 185 * 2);
	notNPb.setPos(15, 60 + 185 * 2);
	andNP.setPos(15, 105 + 185 * 2);
	gateNPs.setPos(15, 150 + 185 * 2);
	sim.connect(less, arrayToStr("a", n - 1), passNPa, "in", inin);
	sim.connect(less, arrayToStr("b", n - 1), notNPb, "in", inin);
	sim.connect(passNPa, "out", andNP, "a");
	sim.connect(notNPb, "out", andNP, "b");
	sim.connect(delay, "out", gateNPs, "a");
	sim.connect(andNP, "out", gateNPs, "b");
	// signal is always gonna be on, this path always returns true for less
	sim.connect(gateNPs, "out", less, "less", outout); 
	// no need to return false for greater
	sim.connect(gateNPs, "out", less, "sigout", outout);

	// Negative negative
	// Negate, then compare, then swap outputs
	// Negation is slow bc of the add1, but this happens to both sides,
	// so we can skip the add1, only doing the invert bits

	// also here is much slower than mixed signs, so skip the 1 cycle delay before the and
	LogicBlock& lessUnsNN = createNBitCompareUns(sim, less, n);
	lessUnsNN.setExpand(false);
	lessUnsNN.setPos(200, 0);

	LogicBlock& andNN = less.add(LogicBlockAnd);
	LogicBlock& gateNNa = createNBitGate(sim, less, n);
	gateNNa.setExpand(false);
	LogicBlock& gateNNb = createNBitGate(sim, less, n);
	gateNNb.setExpand(false);
	LogicBlock& gateNNs = less.add(LogicBlockAnd);
	LogicBlock& negatea = createNBitInvert(sim, less, n);
	LogicBlock& negateb = createNBitInvert(sim, less, n);
	LogicBlock& negatedelay = less.add(LogicBlockDelay);
	andNN.setPos(15, 105 + 185 * 3);
	gateNNs.setPos(15, 150 + 185 * 3);
	gateNNa.setPos(100, 15 + 185 * 3);
	gateNNb.setPos(100, 100 + 185 * 3);
	sim.connect(less, arrayToStr("a", n - 1), andNN, "a", inin);
	sim.connect(less, arrayToStr("b", n - 1), andNN, "b", inin);
	sim.connect(andNN, "out", gateNNa, "write");
	sim.connect(andNN, "out", gateNNb, "write");

	sim.connect(less, "a[]", negatea, "in[]", inin);
	sim.connect(less, "b[]", negateb, "in[]", inin);

	sim.connect(negatea, "out[]", gateNNa, "in[]");
	sim.connect(negateb, "out[]", gateNNb, "in[]");
	sim.connect(gateNNs, "out", negatedelay, "in");
	sim.connect(negatedelay, "out", lessUnsNN, "sigin");
	sim.connect(gateNNa, "out[]", lessUnsNN, "a[]");
	sim.connect(gateNNb, "out[]", lessUnsNN, "b[]");
	sim.connect(delay, "out", gateNNs, "a");
	sim.connect(andNN, "out", gateNNs, "b");
	// swap less/greater bc of negation
	sim.connect(lessUnsNN, "greater", less, "less", outout);
	sim.connect(lessUnsNN, "less", less, "greater", outout);
	sim.connect(lessUnsNN, "sigout", less, "sigout", outout);

	return less;
}


// 2n*log2(n) + 2 cycles
// in: a[], b[], sigin
// out: out[]x2, sigout
static LogicBlock& createNBitMulUns(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& mul = parent.add(LogicBlock(
		arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin", 
		arrayToStr("out", 2 * n) + "|sigout", std::to_string(n) + "bitMulUns"));

	LogicBlock& delay = createNDelay(sim, mul, 2);
	delay.setExpand(false);
	delay.setPos(225, 15);
	sim.connect(mul, "sigin", delay, "in", inin);

	std::vector<LogicBlock*> adds;
	int tempn = (n + 1) / 2;
	int start = 0;
	for (int c = 0; tempn; ++c)
	{
		for (int i = 0; i < tempn; ++i)
		{
			LogicBlock& add = createNBitAdd(sim, mul, n * 2);
			add.setExpand(false);
			add.setPos(300 + c * 125, 50 + i * 280);

			if (c == 0)	// hook up shifts later (0th iteration of c)
			{
				if (i == 0)
					sim.connect(delay, "out", add, "sigin");
			}
			else if (c == 1) // different bc there may be a weird number of adds due to odd n
			{
				if (i == 0)
					sim.connect(*adds[start + i * 2 + 0], "sigout", add, "sigin");
				sim.connect(*adds[start + i * 2 + 0], "out[]", add, "a[]");
				if (i * 2 + 1 < adds.size())
					sim.connect(*adds[start + i * 2 + 1], "out[]", add, "b[]");
			}
			else
			{
				if (i == 0)
					sim.connect(*adds[start + i * 2 + 0], "sigout", add, "sigin");
				sim.connect(*adds[start + i * 2 + 0], "out[]", add, "a[]");
				sim.connect(*adds[start + i * 2 + 1], "out[]", add, "b[]");
			}

			adds.push_back(&add);
		}
		start = adds.size() - tempn;
		tempn /= 2;
	}

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& gate = createNBitGate(sim, mul, n);
		LogicBlock& shift = createNBitShiftLeftM(sim, mul, n * 2, i);

		gate.setExpand(false);
		gate.setPos(50, 50 + i * 150);
		shift.setExpand(false);
		shift.setPos(150, 50 + i * 150);

		sim.connect(mul, "a[]", gate, "in[]", inin);
		sim.connect(mul, arrayToStr("b", i), gate, "write", inin);
		sim.connect(gate, "out[]", shift, "in[]");
		sim.connect(shift, "out[]", *adds[i / 2], (i % 2 == 0 ? "a[]" : "b[]"));
	}

	sim.connect(*adds.back(), "out[]", mul, "out[]", outout);
	sim.connect(*adds.back(), "sigout", mul, "sigout", outout);

	return mul;
}

// 2n*(log2(n) + 1) + 2 cycles
// in: a[], b[], sigin
// out: out[]x2, sigout
static LogicBlock& createNBitMulPN(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& mul = parent.add(LogicBlock(
		arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		arrayToStr("out", 2 * n) + "|sigout", std::to_string(n) + "bitMulPN"));

	LogicBlock& delay = createNDelay(sim, mul, 2);
	delay.setExpand(false);
	delay.setPos(225, 15);
	sim.connect(mul, "sigin", delay, "in", inin);

	std::vector<LogicBlock*> adds;
	int tempn = n / 2;
	int start = 0;
	int c = 0;
	for (; tempn; ++c)
	{
		for (int i = 0; i < tempn; ++i)
		{
			LogicBlock& add = createNBitAdd(sim, mul, n * 2);
			add.setExpand(false);
			add.setPos(300 + c * 125, 50 + i * 280);

			if (c == 0)	// hook up shifts later (0th iteration of c)
			{
				if (i == 0)
					sim.connect(delay, "out", add, "sigin");
			}
			else if (c == 1) // different bc there may be a weird number of adds due to odd n
			{
				if (i == 0)
					sim.connect(*adds[start + i * 2 + 0], "sigout", add, "sigin");
				sim.connect(*adds[start + i * 2 + 0], "out[]", add, "a[]");
				if (i * 2 + 1 < adds.size())
					sim.connect(*adds[start + i * 2 + 1], "out[]", add, "b[]");
			}
			else
			{
				if (i == 0)
					sim.connect(*adds[start + i * 2 + 0], "sigout", add, "sigin");
				sim.connect(*adds[start + i * 2 + 0], "out[]", add, "a[]");
				sim.connect(*adds[start + i * 2 + 1], "out[]", add, "b[]");
			}

			adds.push_back(&add);
		}
		start = adds.size() - tempn;
		tempn /= 2;
	}

	LogicBlock& negate = createNBitNegate(sim, mul, n * 2);

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& gate = createNBitGate(sim, mul, n);
		LogicBlock& shift = createNBitShiftLeftM(sim, mul, n * 2, i);

		gate.setExpand(false);
		gate.setPos(50, 50 + i * 150);
		shift.setExpand(false);
		shift.setPos(150, 50 + i * 150);

		sim.connect(mul, "a[]", gate, "in[]", inin);
		sim.connect(mul, arrayToStr("b", i), gate, "write", inin);
		sim.connect(gate, "out[]", shift, "in[]");
		if (i == n - 1)
			sim.connect(shift, "out[]", negate, "in[]");
		else
			sim.connect(shift, "out[]", *adds[i / 2], (i % 2 == 0 ? "a[]" : "b[]"));
	}

	LogicBlock& delaybuf = createNDelayBuf(sim, mul, n, n * 2);
	LogicBlock& lastAdd = createNBitAdd(sim, mul, n * 2);

	delaybuf.setExpand(false);
	delaybuf.setPos(300 + (c - 1) * 125, 330);
	lastAdd.setExpand(false);
	lastAdd.setPos(300 + c * 125, 50);
	negate.setExpand(false);
	negate.setPos(300, 50 + (n / 2) * 280);

	sim.connect(negate, "out[]", delaybuf, "in[]");
	sim.connect(delaybuf, "out[]", lastAdd, "b[]");
	sim.connect(*adds.back(), "out[]", lastAdd, "a[]");
	sim.connect(*adds.back(), "sigout", lastAdd, "sigin");

	sim.connect(lastAdd, "out[]", mul, "out[]", outout);
	sim.connect(lastAdd, "sigout", mul, "sigout", outout);

	return mul;
}


// 2n*log2(n) + 4(pp) or 2n*(log2(n) + 1) + 5(pn) or 2n*log2(n) + n + 5(nn) cycles
// in: a[], b[], sigin
// out: out[]x2, sigout
static LogicBlock& createNBitMul(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& mul = parent.add(LogicBlock(
		arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin", 
		arrayToStr("out", n * 2) + "|sigout",
		std::to_string(n) + "bitMul"));

	LogicBlock& mulPP = createNBitMulUns(sim, mul, n);
	mulPP.setExpand(false);
	mulPP.setPos(300, 15);
	LogicBlock& mulPN = createNBitMulPN(sim, mul, n);
	mulPN.setExpand(false);
	mulPN.setPos(300, 300);
	LogicBlock& negatea = createNBitNegate(sim, mul, n);
	negatea.setExpand(false);
	negatea.setPos(200, 15);
	LogicBlock& negateb = createNBitNegate(sim, mul, n);
	negateb.setExpand(false);
	negateb.setPos(200, 150);

	LogicBlock& delay = createNDelay(sim, mul, 2);
	delay.setExpand(false);
	sim.connect(mul, "sigin", delay, "in", inin);

	// Positive * positive
	LogicBlock& andPP = mul.add(LogicBlockNor);
	LogicBlock& gatePPa = createNBitGate(sim, mul, n);
	gatePPa.setExpand(false);
	LogicBlock& gatePPb = createNBitGate(sim, mul, n);
	gatePPb.setExpand(false);
	LogicBlock& gatePPs = mul.add(LogicBlockAnd);
	andPP.setPos(15, 105);
	gatePPs.setPos(15, 150);
	gatePPa.setPos(100, 15);
	gatePPb.setPos(100, 100);
	sim.connect(mul, arrayToStr("a", n - 1), andPP, "a", inin);
	sim.connect(mul, arrayToStr("b", n - 1), andPP, "b", inin);
	sim.connect(andPP, "out", gatePPa, "write");
	sim.connect(andPP, "out", gatePPb, "write");
	sim.connect(mul, "a[]", gatePPa, "in[]", inin);
	sim.connect(mul, "b[]", gatePPb, "in[]", inin);
	sim.connect(gatePPa, "out[]", mulPP, "a[]");
	sim.connect(gatePPb, "out[]", mulPP, "b[]");
	sim.connect(delay, "out", gatePPs, "a");
	sim.connect(andPP, "out", gatePPs, "b");
	sim.connect(gatePPs, "out", mulPP, "sigin");
	sim.connect(mulPP, "out[]", mul, "out[]", outout);
	sim.connect(mulPP, "sigout", mul, "sigout", outout);

	// Positive * negative
	LogicBlock& notPNa = mul.add(LogicBlockNot);
	LogicBlock& passPNb = mul.add(LogicBlockDelay);
	LogicBlock& andPN = mul.add(LogicBlockAnd);
	LogicBlock& gatePNa = createNBitGate(sim, mul, n);
	gatePNa.setExpand(false);
	LogicBlock& gatePNb = createNBitGate(sim, mul, n);
	gatePNb.setExpand(false);
	LogicBlock& gatePNs = mul.add(LogicBlockAnd);
	notPNa.setPos(15, 15 + 185);
	passPNb.setPos(15, 60 + 185);
	andPN.setPos(15, 105 + 185);
	gatePNs.setPos(15, 150 + 185);
	gatePNa.setPos(100, 15 + 185);
	gatePNb.setPos(100, 100 + 185);
	sim.connect(mul, arrayToStr("a", n - 1), notPNa, "in", inin);
	sim.connect(mul, arrayToStr("b", n - 1), passPNb, "in", inin);
	sim.connect(notPNa, "out", andPN, "a");
	sim.connect(passPNb, "out", andPN, "b");
	sim.connect(andPN, "out", gatePNa, "write");
	sim.connect(andPN, "out", gatePNb, "write");
	sim.connect(mul, "a[]", gatePNa, "in[]", inin);
	sim.connect(mul, "b[]", gatePNb, "in[]", inin);
	sim.connect(gatePNa, "out[]", mulPN, "a[]");
	sim.connect(gatePNb, "out[]", mulPN, "b[]");
	sim.connect(delay, "out", gatePNs, "a");
	sim.connect(andPN, "out", gatePNs, "b");
	sim.connect(gatePNs, "out", mulPN, "sigin");
	sim.connect(mulPN, "out[]", mul, "out[]", outout);
	sim.connect(mulPN, "sigout", mul, "sigout", outout);

	// Negative * positive
	LogicBlock& passNPa = mul.add(LogicBlockDelay);
	LogicBlock& notNPb = mul.add(LogicBlockNot);
	LogicBlock& andNP = mul.add(LogicBlockAnd);
	LogicBlock& gateNPa = createNBitGate(sim, mul, n);
	gateNPa.setExpand(false);
	LogicBlock& gateNPb = createNBitGate(sim, mul, n);
	gateNPb.setExpand(false);
	LogicBlock& gateNPs = mul.add(LogicBlockAnd);
	passNPa.setPos(15, 15 + 185 * 2);
	notNPb.setPos(15, 60 + 185 * 2);
	andNP.setPos(15, 105 + 185 * 2);
	gateNPs.setPos(15, 150 + 185 * 2);
	gateNPa.setPos(100, 15 + 185 * 2);
	gateNPb.setPos(100, 100 + 185 * 2);
	sim.connect(mul, arrayToStr("a", n - 1), passNPa, "in", inin);
	sim.connect(mul, arrayToStr("b", n - 1), notNPb, "in", inin);
	sim.connect(passNPa, "out", andNP, "a");
	sim.connect(notNPb, "out", andNP, "b");
	sim.connect(andNP, "out", gateNPa, "write");
	sim.connect(andNP, "out", gateNPb, "write");
	sim.connect(mul, "a[]", gateNPa, "in[]", inin);
	sim.connect(mul, "b[]", gateNPb, "in[]", inin);
	sim.connect(gateNPa, "out[]", mulPN, "b[]");
	sim.connect(gateNPb, "out[]", mulPN, "a[]");
	sim.connect(delay, "out", gateNPs, "a");
	sim.connect(andNP, "out", gateNPs, "b");
	sim.connect(gateNPs, "out", mulPN, "sigin");
	sim.connect(mulPN, "out[]", mul, "out[]", outout);
	sim.connect(mulPN, "sigout", mul, "sigout", outout);

	// Negative * negative
	LogicBlock& andNN = mul.add(LogicBlockAnd);
	LogicBlock& gateNNa = createNBitGate(sim, mul, n);
	gateNNa.setExpand(false);
	LogicBlock& gateNNb = createNBitGate(sim, mul, n);
	gateNNb.setExpand(false);
	LogicBlock& gateNNs = mul.add(LogicBlockAnd);
	andNN.setPos(15, 105 + 185 * 3);
	gateNNs.setPos(15, 150 + 185 * 3);
	gateNNa.setPos(100, 15 + 185 * 3);
	gateNNb.setPos(100, 100 + 185 * 3);
	sim.connect(mul, arrayToStr("a", n - 1), andNN, "in", inin);
	sim.connect(mul, arrayToStr("b", n - 1), andNN, "in", inin);
	sim.connect(andNN, "out", gateNNa, "write");
	sim.connect(andNN, "out", gateNNb, "write");
	sim.connect(mul, "a[]", gateNNa, "in[]", inin);
	sim.connect(mul, "b[]", gateNNb, "in[]", inin);
	sim.connect(gateNNa, "out[]", negatea, "in[]");
	sim.connect(gateNNb, "out[]", negateb, "in[]");
	sim.connect(gateNNs, "out", negatea, "sigin");
	sim.connect(negatea, "sigout", mulPP, "sigin");
	sim.connect(negatea, "out[]", mulPP, "a[]");
	sim.connect(negateb, "out[]", mulPP, "b[]");
	sim.connect(delay, "out", gateNNs, "a");
	sim.connect(andNN, "out", gateNNs, "b");
	sim.connect(mulPP, "out[]", mul, "out[]", outout);
	sim.connect(mulPP, "sigout", mul, "sigout", outout);

	return mul;
}


// n cycles on, n cycles off
// in: pause
// out: out
static LogicBlock& createClockN(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& clock = parent.add(LogicBlock("pause", "out", std::to_string(n) + "cycleClock"));

	LogicBlock& tempnot = clock.add(LogicBlockNot);
	tempnot.setPos(5, 5);
	LogicBlock* ptr = &tempnot;
	sim.connect(tempnot, "out", clock, "out", outout);
	sim.connect(clock, "pause", tempnot, "in", inin);

	for (int i = 1; i < n; ++i)
	{
		LogicBlock& delay = clock.add(LogicBlockDelay);
		delay.setPos(5, i * 25);
		sim.connect(*ptr, "out", delay, "in");
		ptr = &delay;
	}

	sim.connect(*ptr, "out", tempnot, "in");

	return clock;
}

// 2 cycles, n cycles on
// in: in
// out: out
static LogicBlock& createNPulse(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& pulse = parent.add(LogicBlock("in", "out", std::to_string(n) + "pulse"));

	LogicBlock& tempdelay = pulse.add(LogicBlockDelay);
	LogicBlock& tempxor = pulse.add(LogicBlockXor);
	LogicBlock& tempand = pulse.add(LogicBlockAnd);

	tempdelay.setPos(15, 15);
	tempxor.setPos(90, 15);
	tempand.setPos(165, 15);

	sim.connect(pulse, "in", tempdelay, "in", inin);
	sim.connect(pulse, "in", tempxor, "a", inin);
	sim.connect(tempdelay, "out", tempxor, "b");
	sim.connect(tempxor, "out", tempand, "a");
	sim.connect(tempdelay, "out", tempand, "b");
	sim.connect(tempand, "out", pulse, "out", outout);

	LogicBlock* ptr = NULL;
	for (int i = 0; i < n - 1; ++i)
	{
		LogicBlock& temp = pulse.add(LogicBlockDelay);

		if (ptr)
			sim.connect(*ptr, "out", temp, "in");
		else
			sim.connect(tempand, "out", temp, "in");
		sim.connect(temp, "out", pulse, "out", outout);

		ptr = &temp;
	}

	return pulse;
}

// 4 cycle write
// 0 cycle read
// in: data, write
// out: out
static LogicBlock& createMemoryCell(LogicSim& sim, LogicBlock& parent)
{
	LogicBlock& mem = parent.add(LogicBlock("data|write", "out", "memoryCell"));

	LogicBlock& inv = mem.add(LogicBlockNot);
	LogicBlock& pass = mem.add(LogicBlockDelay);
	inv.setPos(15, 15);
	pass.setPos(15, 50);
	sim.connect(mem, "data", inv, "in", inin);
	sim.connect(mem, "data", pass, "in", inin);

	LogicBlock& gate = createNBitGate(sim, mem, 2);
	gate.setPos(80, 15);
	sim.connect(inv, "out", gate, "in[0]");
	sim.connect(pass, "out", gate, "in[1]");
	sim.connect(mem, "write", gate, "write", inin);

	LogicBlock& stab = mem.add(LogicBlockAnd);
	stab.setPos(175, 15);

	LogicBlock& delay0 = mem.add(LogicBlockDelay);
	delay0.setPos(175, 50);
	LogicBlock& delay1 = mem.add(LogicBlockDelay);
	delay1.setPos(175, 75);

	LogicBlock& tempnot0 = mem.add(LogicBlockNot);
	LogicBlock& tempnot1 = mem.add(LogicBlockNot);
	tempnot0.setPos(250, 15);
	tempnot1.setPos(250, 50);

	sim.connect(gate, "out[0]", tempnot0, "in");
	sim.connect(gate, "out[1]", tempnot1, "in");

	sim.connect(gate, "out[0]", delay0, "in");
	sim.connect(gate, "out[1]", delay1, "in");
	sim.connect(delay0, "out", tempnot0, "in");
	sim.connect(delay1, "out", tempnot1, "in");

	sim.connect(tempnot0, "out", tempnot1, "in");
	sim.connect(tempnot1, "out", tempnot0, "in");

	sim.connect(tempnot0, "out", stab, "a");
	sim.connect(tempnot1, "out", stab, "b");
	sim.connect(stab, "out", tempnot0, "in");

	sim.connect(tempnot0, "out", mem, "out", outout);

	return mem;
}

// 5 cycle write
// 1 cycle read
// in: data[], write, read
// out: out[], sigout
static LogicBlock& createNBitMemory(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& mem = parent.add(LogicBlock(arrayToStr("data", n) + "|write|read", 
		arrayToStr("out", n) + "|sigout", std::to_string(n) + "bitMemory"));

	LogicBlock& gate = createNBitGate(sim, mem, n);
	sim.connect(mem, "read", gate, "write", inin);
	LogicBlock& readdelay = mem.add(LogicBlockDelay);
	sim.connect(mem, "read", readdelay, "in", inin);
	sim.connect(readdelay, "out", mem, "sigout", outout);

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& temp = createMemoryCell(sim, mem);
		temp.setExpand(false);
		temp.setPos(15, 15 + i * 50);
		sim.connect(mem, arrayToStr("data", i), temp, "data", inin);
		sim.connect(mem, "write", temp, "write", inin);
		sim.connect(temp, "out", gate, arrayToStr("in", i));
	}

	sim.connect(gate, "out[]", mem, "out[]", outout);
	gate.setPos(200, 15);

	LogicBlock& delay = createNDelay(sim, mem, 5);
	sim.connect(mem, "write", delay, "in", inin);
	sim.connect(delay, "out", mem, "sigout", outout);

	return mem;
}

// 8 cycles write
// 4 cycles read
// in: data[], addr[], write, read
// out: out[], sigout
static LogicBlock& createAddrMemory(LogicSim& sim, LogicBlock& parent, int addrBits, int bitsPerByte, int n)
{
	LogicBlock& addr = parent.add(LogicBlock(
		arrayToStr("data", bitsPerByte) + "|" + arrayToStr("addr", addrBits) + 
		"|write|read", arrayToStr("out", bitsPerByte) + "|sigout", 
		std::to_string(addrBits) + "bitAddrMem"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& mem = createNBitMemory(sim, addr, bitsPerByte);
		LogicBlock& cmp = createNBitIsEqual(sim, addr, addrBits);
		LogicBlock& num = createNBitNumber(sim, addr, addrBits, i);
		LogicBlock& gate = createNBitGate(sim, addr, 2);
		LogicBlock& readdelay = createNDelay(sim, addr, 4);
		LogicBlock& writedelay = createNDelay(sim, addr, 4);
		LogicBlock& datadelay = createNDelayBuf(sim, addr, 3, bitsPerByte);

		mem.setExpand(false);
		cmp.setExpand(false);
		gate.setExpand(false);
		readdelay.setExpand(false);
		writedelay.setExpand(false);
		datadelay.setExpand(false);

		num.setPos(50, 15 + i * 200);
		cmp.setPos(150, 15 + i * 200);
		writedelay.setPos(50, 100 + i * 200);
		readdelay.setPos(50, 140 + i * 200);
		gate.setPos(250, 125 + i * 200);
		mem.setPos(250, 15 + i * 200);
		datadelay.setPos(15, 15 + i * 200);

		sim.connect(num, "out[]", cmp, "b[]");
		sim.connect(addr, "addr[]", cmp, "a[]", inin);
		sim.connect(cmp, "out", gate, "write");
		sim.connect(addr, "write", writedelay, "in", inin);
		sim.connect(addr, "read", readdelay, "in", inin);
		sim.connect(writedelay, "out", gate, "in[0]");
		sim.connect(readdelay, "out", gate, "in[1]");

		sim.connect(gate, "out[0]", mem, "write");
		sim.connect(gate, "out[1]", mem, "read");
		sim.connect(addr, "data[]", datadelay, "in[]", inin);
		sim.connect(datadelay, "out[]", mem, "data[]");

		sim.connect(mem, "out[]", addr, "out[]", outout);
		sim.connect(mem, "sigout", addr, "sigout", outout);
	}

	return addr;
}


// 1 cycle read
// in: read
// out: out[]
static LogicBlock& createNBitROM(LogicSim& sim, LogicBlock& parent, int n, int64_t data)
{
	LogicBlock& rom = parent.add(LogicBlock("read", arrayToStr("out", n), 
		std::to_string(n) + "bitROM"));

	LogicBlock& num = createNBitNumber(sim, rom, n, data);
	LogicBlock& gate = createNBitGate(sim, rom, n);
	sim.connect(num, "out[]", gate, "in[]");
	sim.connect(rom, "read", gate, "write", inin);
	sim.connect(gate, "out[]", rom, "out[]", outout);

	return rom;
}

// 4 cycles read
// in: addr[], read
// out: out[], sigout
static LogicBlock& createAddrROM(LogicSim& sim, LogicBlock& parent, int addrBits, int bitsPerByte, int n, int64_t* data)
{
	LogicBlock& addr = parent.add(LogicBlock(
		arrayToStr("addr", addrBits) + "|read", 
		arrayToStr("out", bitsPerByte) + "|sigout",
		std::to_string(addrBits) + "bitAddrROM"));

	LogicBlock& readdelay = createNDelay(sim, addr, 5);
	sim.connect(addr, "read", readdelay, "in", inin);
	sim.connect(readdelay, "out", addr, "sigout", outout);
	readdelay.setExpand(false);

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& mem = createNBitROM(sim, addr, bitsPerByte, data[i]);
		LogicBlock& cmp = createNBitIsEqual(sim, addr, addrBits);
		LogicBlock& num = createNBitNumber(sim, addr, addrBits, i);
		LogicBlock& tempand = addr.add(LogicBlockAnd);
		LogicBlock& delay = createNDelay(sim, addr, 2);

		mem.setExpand(false);
		cmp.setExpand(false);
		delay.setExpand(false);

		num.setPos(50, 15 + i * 200);
		cmp.setPos(150, 15 + i * 200);
		mem.setPos(250, 15 + i * 200);

		sim.connect(num, "out[]", cmp, "b[]"); // 0
		sim.connect(addr, "addr[]", cmp, "a[]", inin); // 0

		sim.connect(cmp, "out", tempand, "a"); // 2
		sim.connect(addr, "read", delay, "in", inin); // 0
		sim.connect(delay, "out", tempand, "b"); // 2
		sim.connect(tempand, "out", mem, "read"); // 3

		sim.connect(mem, "out[]", addr, "out[]", outout); // 4

	}

	return addr;
}

// n + 7 or 7 cycles write
// in: data[], inc, set
// out: data[], sigout
static LogicBlock& createProgramCounter(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& pc = parent.add(LogicBlock(arrayToStr("data", n) + "|inc|set",
		arrayToStr("data", n) + "|sigout", std::to_string(n) + "bitPC"));

	LogicBlock& mem = createNBitMemory(sim, pc, n);
	LogicBlock& read = pc.add(LogicBlockOn);
	sim.connect(read, "out", mem, "read");

	LogicBlock& setGate = createNBitGate(sim, pc, n);
	LogicBlock& setDelay = createNDelay(sim, pc, 1); // somehow this is different than a regular delay

	LogicBlock& inc = createNBitAdd1(sim, pc, n);
	LogicBlock& incGate = createNBitGate(sim, pc, n);
	LogicBlock& incDelay = pc.add(LogicBlockDelay);
	
	LogicBlock& sigdelay = createNDelay(sim, pc, 6);

	sim.connect(mem, "out[]", inc, "a[]");
	sim.connect(pc, "inc", inc, "sigin", inin);
	sim.connect(inc, "out[]", incGate, "in[]");
	sim.connect(inc, "sigout", incGate, "write");
	sim.connect(inc, "sigout", incDelay, "in");
	sim.connect(incGate, "out[]", mem, "data[]");
	sim.connect(incDelay, "out", mem, "write");
	sim.connect(incDelay, "out", sigdelay, "in");

	sim.connect(pc, "data[]", setGate, "in[]", inin);
	sim.connect(pc, "set", setGate, "write", inin);
	sim.connect(pc, "set", setDelay, "in", inin);
	sim.connect(setGate, "out[]", mem, "data[]");
	sim.connect(setDelay, "out", mem, "write");
	sim.connect(setDelay, "out", sigdelay, "in");

	// always read enabled, so sigout can't come from mem
	// write is 5 cycles
	sim.connect(sigdelay, "out", pc, "sigout", outout);
	sim.connect(mem, "out[]", pc, "data[]", outout);

	return pc;
}

// 4 cycles write
// in: zwrite, zdata, ewrite, edata, lwrite, ldata, gwrite, gdata, owrite, odata
// out: zero, equal, less, greater, overflow
static LogicBlock& createFlagRegister(LogicSim& sim, LogicBlock& parent)
{
	LogicBlock& flags = parent.add(LogicBlock(
		"zwrite|zdata|ewrite|edata|lwrite|ldata|gwrite|gdata|owrite|odata", 
		"zero|equal|less|greater|overflow", "flagRegister"));

	LogicBlock& zero = createMemoryCell(sim, flags);
	LogicBlock& equal = createMemoryCell(sim, flags);
	LogicBlock& less = createMemoryCell(sim, flags);
	LogicBlock& greater = createMemoryCell(sim, flags);
	LogicBlock& overflow = createMemoryCell(sim, flags);

	sim.connect(flags, "zwrite", zero, "write", inin);
	sim.connect(flags, "zdata", zero, "data", inin);
	sim.connect(zero, "out", flags, "zero", outout);

	sim.connect(flags, "ewrite", equal, "write", inin);
	sim.connect(flags, "edata", equal, "data", inin);
	sim.connect(equal, "out", flags, "equal", outout);

	sim.connect(flags, "lwrite", less, "write", inin);
	sim.connect(flags, "ldata", less, "data", inin);
	sim.connect(less, "out", flags, "less", outout);

	sim.connect(flags, "gwrite", greater, "write", inin);
	sim.connect(flags, "gdata", greater, "data", inin);
	sim.connect(greater, "out", flags, "greater", outout);

	sim.connect(flags, "owrite", overflow, "write", inin);
	sim.connect(flags, "odata", overflow, "data", inin);
	sim.connect(overflow, "out", flags, "overflow", outout);

	return flags;
}


// in: one, five, ten, addsub, action
// out: out[], sigout
static LogicBlock& createNBitNumberEditor(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& edit = parent.add(LogicBlock("one|five|ten|addsub|action", 
		arrayToStr("out", n) + "|sigout",
		std::to_string(n) + "bitNumEdit"));

	LogicBlock& mem = createNBitMemory(sim, edit, n);
	LogicBlock& on = edit.add(LogicBlockOn);

	LogicBlock& add = createNBitAdd(sim, edit, n);
	LogicBlock& sub = createNBitSub(sim, edit, n);

	LogicBlock& one = createNBitNumber(sim, edit, n, 1);
	LogicBlock& onegate = createNBitGate(sim, edit, n);

	LogicBlock& fiv = createNBitNumber(sim, edit, n, 5);
	LogicBlock& fivgate = createNBitGate(sim, edit, n);

	LogicBlock& ten = createNBitNumber(sim, edit, n, 10);
	LogicBlock& tengate = createNBitGate(sim, edit, n);

	LogicBlock& addsubinv = edit.add(LogicBlockNot);
	LogicBlock& addgate = createNBitGate(sim, edit, n);
	LogicBlock& subgate = createNBitGate(sim, edit, n);

	one.setPos(15, 15);
	onegate.setPos(100, 15);
	onegate.setExpand(false);
	sim.connect(one, "out[]", onegate, "in[]");
	sim.connect(edit, "one", onegate, "write", inin);
	sim.connect(onegate, "out[]", add, "b[]");
	sim.connect(onegate, "out[]", sub, "b[]");

	fiv.setPos(15, 125);
	fivgate.setPos(100, 125);
	fivgate.setExpand(false);
	sim.connect(fiv, "out[]", fivgate, "in[]");
	sim.connect(edit, "five", fivgate, "write", inin);
	sim.connect(fivgate, "out[]", add, "b[]");
	sim.connect(fivgate, "out[]", sub, "b[]");

	ten.setPos(15, 235);
	tengate.setPos(100, 235);
	tengate.setExpand(false);
	sim.connect(ten, "out[]", tengate, "in[]");
	sim.connect(edit, "ten", tengate, "write", inin);
	sim.connect(tengate, "out[]", add, "b[]");
	sim.connect(tengate, "out[]", sub, "b[]");

	sim.connect(edit, "action", mem, "write", inin);

	addsubinv.setPos(75, 350);
	addgate.setPos(250, 100);
	addgate.setExpand(false);
	subgate.setPos(250, 200);
	subgate.setExpand(false);
	sim.connect(edit, "addsub", addsubinv, "in", inin);
	sim.connect(edit, "addsub", addgate, "write", inin);
	sim.connect(addsubinv, "out", subgate, "write");
	sim.connect(addgate, "out[]", mem, "data[]");
	sim.connect(subgate, "out[]", mem, "data[]");

	mem.setPos(350, 15);
	mem.setExpand(false);
	on.setPos(300, 15);
	sim.connect(on, "out", mem, "read");

	add.setPos(500, 15);
	add.setExpand(false);
	sub.setPos(500, 200);
	sub.setExpand(false);

	sim.connect(mem, "out[]", edit, "out[]", outout);

	sim.connect(mem, "out[]", add, "a[]");
	sim.connect(add, "out[]", addgate, "in[]");
	sim.connect(mem, "out[]", sub, "a[]");
	sim.connect(sub, "out[]", subgate, "in[]");

	LogicBlock& writedelay = createNDelay(sim, edit, 6);
	sim.connect(edit, "action", writedelay, "in", inin);
	sim.connect(writedelay, "out", edit, "sigout", outout);

	return edit;
}

// out: out[], sigout
static LogicBlock& createNBitNumberEditorCtrl(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& ctrl = parent.add(LogicBlock("", arrayToStr("out", n) + "|sigout",
		std::to_string(n) + "bitNumEditCtrl"));

	LogicBlock& edit = createNBitNumberEditor(sim, ctrl, 8);
	edit.setExpand(false);
	edit.setPos(75, 15);
	LogicBlock& one = ctrl.add(LogicBlockToggle);
	LogicBlock& five = ctrl.add(LogicBlockToggle);
	LogicBlock& ten = ctrl.add(LogicBlockToggle);
	LogicBlock& addsub = ctrl.add(LogicBlockToggle);
	LogicBlock& action = ctrl.add(LogicBlockButton);
	sim.connect(one, "out", edit, "one");
	sim.connect(five, "out", edit, "five");
	sim.connect(ten, "out", edit, "ten");
	sim.connect(addsub, "out", edit, "addsub");
	sim.connect(action, "out", edit, "action");
	one.setPos(15, 15);
	five.setPos(15, 50);
	ten.setPos(15, 95);
	addsub.setPos(15, 140);
	action.setPos(15, 185);

	sim.connect(edit, "out[]", ctrl, "out[]", outout);
	sim.connect(edit, "sigout", ctrl, "sigout", outout);

	return ctrl;
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

static void drawNumberLabel(SDL_Surface* surface, LogicBlock& block, int n)
{
	XYCoord coord = block.getConnectionPos(0, false);
	int64_t value = 0;
	if (block.output.size())
		value = readNBitInput(block, n);
	else
		value = readNBitOutput(block, n);
	drawTextFA(surface, coord.x - font_w / 2, coord.y - font_h * 3 / 2, 1, -1, 0, rgb(255, 255, 255), "%lld", value);
}
