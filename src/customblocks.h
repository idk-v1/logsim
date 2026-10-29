#pragma once

#include "logicsim.h"
#include "../graphics/graphics.h"
#include "../graphics/bulletinV1_font.h"

static LogicBlock& createPowerOn(LogicSim& sim, LogicBlock& parent)
{
	LogicBlock& power = parent.add(LogicBlock("", "out", "on"));

	power.output["out"].data = true;

	return power;
}

static LogicBlock& createPowerOff(LogicSim& sim, LogicBlock& parent)
{
	LogicBlock& power = parent.add(LogicBlock("", "out", "off"));

	return power;
}

// n cycles
static LogicBlock& createNDelay(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& delay = parent.add(LogicBlock("in", "out", std::to_string(n) + "delay"));

	LogicBlock* old = NULL;
	for (int i = 0; i < n; ++i)
	{
		LogicBlock& temp = delay.add(LogicBlockOr);
		temp.setPos(15 + (i / 10) * 50, 15 + (i % 10) * 30);
		if (i == 0)
			sim.connect(delay, "in", temp, "a", inin);
		else if (old)
			sim.connect(*old, "out", temp, "a");

		old = &temp;
	}
	if (old)
		sim.connect(*old, "out", delay, "out", outout);
	else
		sim.connect(delay, "in", delay, "out", inout);

	return delay;
}

// 1 cycle
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
static LogicBlock& createNBitAdd1(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& add = parent.add(LogicBlock(arrayToStr("a", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitAdd1"));

	LogicBlock* oldAdd = NULL;
	LogicBlock& one = createPowerOn(sim, add);

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
static LogicBlock& createFullAdd(LogicSim& sim, LogicBlock& parent)
{
	LogicBlock& fullAdd = parent.add(LogicBlock("a|b|c", "out|carry", "fullAdd"));

	LogicBlock& tempxor0 = fullAdd.add(LogicBlockXor);
	LogicBlock& tempand0 = fullAdd.add(LogicBlockAnd);
	LogicBlock& tempxor1 = fullAdd.add(LogicBlockXor);
	LogicBlock& tempand1 = fullAdd.add(LogicBlockAnd);
	LogicBlock& tempdelay0 = createNDelay(sim, fullAdd, 1);
	LogicBlock& tempdelay2 = createNDelay(sim, fullAdd, 1);

	// step 0
	// tempxor0
	// tempand0
	// tempdelay2

	// step 1
	// tempdelay0
	// tempxor1
	// tempand1

	sim.connect(fullAdd, "a", tempxor0, "a", inin); // 0
	sim.connect(fullAdd, "b", tempxor0, "b", inin); // 0

	sim.connect(fullAdd, "a", tempand0, "a", inin); // 0
	sim.connect(fullAdd, "b", tempand0, "b", inin); // 0
	sim.connect(tempand0, "out", tempdelay0, "in"); // 1

	sim.connect(fullAdd, "c", tempdelay2, "in", inin); // 0

	sim.connect(tempxor0, "out", tempxor1, "a"); // 1
	sim.connect(tempdelay2, "out", tempxor1, "b"); // 1
	sim.connect(tempxor1, "out", fullAdd, "out", outout); // 2

	sim.connect(tempxor0, "out", tempand1, "a"); // 1
	sim.connect(tempdelay2, "out", tempand1, "b"); // 1
	sim.connect(tempand1, "out", fullAdd, "carry", outout); // 2
	sim.connect(tempdelay0, "out", fullAdd, "carry", outout); // 2

	return fullAdd;
}

// 2n cycles
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
static LogicBlock& createNBitInvert(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& inv = parent.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n), std::to_string(n) + "bitInvert"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempNot = inv.add(LogicBlockNot);
		sim.connect(inv, arrayToStr("in", i), tempNot, "in", inin);
		sim.connect(tempNot, "out", inv, arrayToStr("out", i), outout);
	}

	return inv;
}

// 3n + 2 cycles
static LogicBlock& createNBitSub(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& sub = parent.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitSubtract"));

	LogicBlock& inv = createNBitInvert(sim, sub, n);
	LogicBlock& tempAdd = createNBitAdd1(sim, sub, n);
	LogicBlock& add = createNBitAdd(sim, sub, n);
	LogicBlock& ret = sub.add(LogicBlockNot);

	LogicBlock& delay0 = createNDelay(sim, sub, 1);
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

	sim.connect(sub, "sigin", delay0, "in", inin);
	sim.connect(delay0, "out", tempAdd, "sigin");
	sim.connect(tempAdd, "sigout", add, "sigin");

	sim.connect(sub, "b[]", inv, "in[]", inin); // 0
	sim.connect(inv, "out[]", tempAdd, "a[]"); // 1

	sim.connect(sub, "a[]", add, "a[]", inin); // 0
	sim.connect(tempAdd, "out[]", add, "b[]"); // 2

	sim.connect(add, "out[]", sub, "out[]", outout); // 3
	sim.connect(add, "overflow", ret, "in"); // 3
	sim.connect(ret, "out", sub, "overflow", outout); // 4

	sim.connect(add, "sigout", sub, "sigout", outout);

	return sub;
}

// 1 cycle
static LogicBlock& createNBitXor(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& nxor = parent.add(LogicBlock(arrayToStr("in", n),
		arrayToStr("out", n), std::to_string(n) + "bitXor"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempXor = nxor.add(LogicBlockXor);
		sim.connect(nxor, arrayToStr("in", i), tempXor, "in", inin);
		sim.connect(tempXor, "out", nxor, arrayToStr("out", i), outout);
	}

	return nxor;
}

// 1 cycle
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
static LogicBlock& createNBitCompare(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& comp = parent.add(LogicBlock(arrayToStr("a", n) + "|" + arrayToStr("b", n),
		"out", std::to_string(n) + "bitCompare"));

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
static LogicBlock& createNBitGate(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& gate = parent.add(LogicBlock(arrayToStr("in", n) + "|power",
		arrayToStr("out", n), std::to_string(n) + "bitGate"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& tempand = gate.add(LogicBlockAnd);
		sim.connect(gate, arrayToStr("in", i), tempand, "a", inin);
		sim.connect(gate, "power", tempand, "b", inin);
		sim.connect(tempand, "out", gate, arrayToStr("out", i), outout);
	}

	return gate;
}

// 1 cycle
static LogicBlock& createNBitShiftM(LogicSim& sim, LogicBlock& parent, int n, int m)
{
	LogicBlock& shift = parent.add(LogicBlock(arrayToStr("in", n), arrayToStr("out", n),
		std::to_string(n) + "bitShift" + (m > 0 ? "Up" : "Down") + std::to_string(abs(m))));

	for (int i = 0; i < n; ++i)
	{
		int index = i + m;
		if (index >= 0 && index < n)
			sim.connect(shift, arrayToStr("in", i), shift, arrayToStr("out", index), inout);
	}

	return shift;
}

// n cycles on, n cycles off
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
		LogicBlock& delay = createNDelay(sim, clock, 1);
		delay.setPos(5, i * 25);
		sim.connect(*ptr, "out", delay, "in");
		ptr = &delay;
	}

	sim.connect(*ptr, "out", tempnot, "in");

	return clock;
}

// 2 cycles
static LogicBlock& createPulse(LogicSim& sim, LogicBlock& parent)
{
	LogicBlock& pulse = parent.add(LogicBlock("in", "out", "pulse"));

	LogicBlock& tempdelay = createNDelay(sim, pulse, 1);
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

	return pulse;
}


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

static LogicBlock& createNBitOutput(LogicSim& sim, LogicBlock& parent, int n)
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
	drawTextFA(surface, coord.x - font_w / 2, coord.y - font_h * 3 / 2, 1, -1, 0, rgb(255, 255, 255), "%lld", value);
}
