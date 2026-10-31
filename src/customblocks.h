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

// 3n + 2 cycles
// in: a[], b[], sigin
// out: out[], overflow, sigout
static LogicBlock& createNBitSub(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& sub = parent.add(LogicBlock(
		arrayToStr("a", n) + "|" + arrayToStr("b", n) + "|sigin",
		arrayToStr("out", n) + "|overflow|sigout", std::to_string(n) + "bitSubtract"));

	LogicBlock& inv = createNBitInvert(sim, sub, n);
	LogicBlock& tempAdd = createNBitAdd1(sim, sub, n);
	LogicBlock& add = createNBitAdd(sim, sub, n);
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

// 1 cycle
// in: in[]
// out: out[]
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
static LogicBlock& createMemoryAddr(LogicSim& sim, LogicBlock& parent, int addrBits, int bitsPerByte, int n)
{
	LogicBlock& addr = parent.add(LogicBlock(
		arrayToStr("data", bitsPerByte) + "|" + arrayToStr("addr", addrBits) + 
		"|write|read", arrayToStr("out", bitsPerByte) + "|sigout", 
		std::to_string(addrBits) + "bitMemAddr"));

	for (int i = 0; i < n; ++i)
	{
		LogicBlock& mem = createNBitMemory(sim, addr, bitsPerByte);
		LogicBlock& cmp = createNBitCompare(sim, addr, addrBits);
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


// in: one, five, ten, addsub, action
// out: out[]
static LogicBlock& createNBitNumberEditor(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& edit = parent.add(LogicBlock("one|five|ten|addsub|action", arrayToStr("out", n), 
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

	return edit;
}

// out: out[]
static LogicBlock& createNBitNumberEditorCtrl(LogicSim& sim, LogicBlock& parent, int n)
{
	LogicBlock& ctrl = parent.add(LogicBlock("", arrayToStr("out", n), 
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
