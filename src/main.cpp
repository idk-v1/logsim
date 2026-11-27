#include "logicsim.h"
#include "customblocks.h"

#include <iostream>

#include "program.h"

int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	int width = 1200, height = 600;
	SDL_Window* window = SDL_CreateWindow("logsim", width, height, SDL_WINDOW_RESIZABLE);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	setPixelFormat(surface);

	LogicSim sim;
	int n = 8;

	LogicBlock& startbtn = sim.add(LogicBlockButton);
	startbtn.setPos(250, 200);

	LogicBlock& instr_rom  = createAddrROM(sim, sim.getBoard(), n, n, sizeof(instr_data) / sizeof(*instr_data), instr_data);
	LogicBlock& param1_rom = createAddrROM(sim, sim.getBoard(), n, n, sizeof(instr_data) / sizeof(*instr_data), param1_data);
	LogicBlock& param2_rom = createAddrROM(sim, sim.getBoard(), n, n, sizeof(instr_data) / sizeof(*instr_data), param2_data);
	LogicBlock& param3_rom = createAddrROM(sim, sim.getBoard(), n, n, sizeof(instr_data) / sizeof(*instr_data), param3_data);
	LogicBlock& registers = createAddrMemory(sim, sim.getBoard(), n, n, 64);

	LogicBlock& flags = createFlagRegister(sim, sim.getBoard());
	registers.setPos(700, 15);
	flags.setPos(550, 15);

	LogicBlock& pc = createProgramCounter(sim, sim.getBoard(), n);
	LogicBlock& romdelay = createNDelay(sim, sim.getBoard(), 5); // idk why this is needed
	romdelay.setPos(300, 500);
	sim.connect(pc, "sigout", romdelay, "in");
	sim.connect(startbtn, "out", romdelay, "in");
	LogicBlock& incbtn = sim.add(LogicBlockButton);
	sim.connect(incbtn, "out", pc, "inc");
	LogicBlock& edit = createNBitNumberEditorCtrl(sim, sim.getBoard(), n);
	LogicBlock& setbtn = sim.add(LogicBlockButton);
	sim.connect(setbtn, "out", pc, "set");
	sim.connect(edit, "out[]", pc, "data[]");

	LogicBlock& readROM = sim.add(LogicBlockOn);
	readROM.setPos(250, 150);
	sim.connect(readROM, "out", instr_rom, "read");
	sim.connect(readROM, "out", param1_rom, "read");
	sim.connect(readROM, "out", param2_rom, "read");
	sim.connect(readROM, "out", param3_rom, "read");

	LogicBlock& led0 = sim.add(LogicBlockLED);
	led0.setPos(350, 150);
	sim.connect(readROM, "out", led0, "in");
	LogicBlock& led1 = sim.add(LogicBlockLED);
	led1.setPos(350+17, 150);
	sim.connect(readROM, "out", led1, "in");

	edit.setExpand(true);
	edit.setPos(15, 15);
	setbtn.setPos(15, 250);
	incbtn.setPos(15, 275);

	pc.setPos(250, 15);
	instr_rom.setPos(400, 15);
	param1_rom.setPos(400, 150);
	param2_rom.setPos(400, 275);
	param3_rom.setPos(400, 400);

	sim.connect(pc, "data[]", instr_rom, "addr[]");
	sim.connect(pc, "data[]", param1_rom, "addr[]");
	sim.connect(pc, "data[]", param2_rom, "addr[]");
	sim.connect(pc, "data[]", param3_rom, "addr[]");

	LogicBlock& addressMode = createAddressMode(sim, sim.getBoard(), n);
	addressMode.setPos(850, 15);
	sim.connect(param1_rom, "out[]", addressMode, "p0[]");
	sim.connect(param2_rom, "out[]", addressMode, "p1[]");
	sim.connect(param3_rom, "out[]", addressMode, "p2[]");
	sim.connect(registers, "out[]", addressMode, "memin[]");
	sim.connect(registers, "rsigout", addressMode, "rsigin");
	sim.connect(romdelay, "out", addressMode, "sigin");
	sim.connect(flags, "am[]", addressMode, "am[]");
	sim.connect(addressMode, "addr[]", registers, "addr[]");
	sim.connect(addressMode, "rsigout", registers, "read");

	
	LogicBlock& nop = createInstrNOP(sim, sim.getBoard(), n);
	nop.setPos(500 + 250 * 0, 500);
	sim.connect(instr_rom, "out[]", nop, "num[]");
	sim.connect(romdelay, "out", nop, "sigin");
	sim.connect(nop, "sigout", pc, "inc");

	LogicBlock& halt = createInstrHalt(sim, sim.getBoard(), n);
	halt.setPos(500 + 250 * 1, 500);
	sim.connect(instr_rom, "out[]", halt, "num[]");
	sim.connect(romdelay, "out", halt, "sigin");

	LogicBlock& jump = createInstrJump(sim, sim.getBoard(), n);
	jump.setPos(500 + 250 * 2, 500);
	sim.connect(instr_rom, "out[]", jump, "num[]");
	sim.connect(addressMode, "p0[]", jump, "addr[]");
	sim.connect(addressMode, "sigout", jump, "sigin");
	sim.connect(jump, "addr[]", pc, "data[]");
	sim.connect(jump, "setout", pc, "set");

	LogicBlock& jumpifzero = createInstrJumpZero(sim, sim.getBoard(), n);
	jumpifzero.setPos(500 + 250 * 3, 500);
	sim.connect(instr_rom, "out[]", jumpifzero, "num[]");
	sim.connect(addressMode, "p0[]", jumpifzero, "addr[]");
	sim.connect(addressMode, "sigout", jumpifzero, "sigin");
	sim.connect(flags, "zero", jumpifzero, "zero");
	sim.connect(jumpifzero, "addr[]", pc, "data[]");
	sim.connect(jumpifzero, "setout", pc, "set");
	sim.connect(jumpifzero, "incout", pc, "inc");

	LogicBlock& jumpifless = createInstrJumpLs(sim, sim.getBoard(), n);
	jumpifless.setPos(500 + 250 * 4, 500);
	sim.connect(instr_rom, "out[]", jumpifless, "num[]");
	sim.connect(addressMode, "p0[]", jumpifless, "addr[]");
	sim.connect(addressMode, "sigout", jumpifless, "sigin");
	sim.connect(flags, "less", jumpifless, "less");
	sim.connect(jumpifless, "addr[]", pc, "data[]");
	sim.connect(jumpifless, "setout", pc, "set");
	sim.connect(jumpifless, "incout", pc, "inc");

	LogicBlock& jumpifgr = createInstrJumpGr(sim, sim.getBoard(), n);
	jumpifgr.setPos(500 + 250 * 5, 500);
	sim.connect(instr_rom, "out[]", jumpifgr, "num[]");
	sim.connect(addressMode, "p0[]", jumpifgr, "addr[]");
	sim.connect(addressMode, "sigout", jumpifgr, "sigin");
	sim.connect(flags, "greater", jumpifgr, "greater");
	sim.connect(jumpifgr, "addr[]", pc, "data[]");
	sim.connect(jumpifgr, "setout", pc, "set");
	sim.connect(jumpifgr, "incout", pc, "inc");

	LogicBlock& jumpifeq = createInstrJumpEq(sim, sim.getBoard(), n);
	jumpifeq.setPos(500 + 250 * 6, 500);
	sim.connect(instr_rom, "out[]", jumpifeq, "num[]");
	sim.connect(addressMode, "p0[]", jumpifeq, "addr[]");
	sim.connect(addressMode, "sigout", jumpifeq, "sigin");
	sim.connect(flags, "equal", jumpifeq, "equal");
	sim.connect(jumpifeq, "addr[]", pc, "data[]");
	sim.connect(jumpifeq, "setout", pc, "set");
	sim.connect(jumpifeq, "incout", pc, "inc");

	LogicBlock& jumpifov = createInstrJumpOv(sim, sim.getBoard(), n);
	jumpifov.setPos(500 + 250 * 7, 500);
	sim.connect(instr_rom, "out[]", jumpifov, "num[]");
	sim.connect(addressMode, "p0[]", jumpifov, "addr[]");
	sim.connect(addressMode, "sigout", jumpifov, "sigin");
	sim.connect(flags, "overflow", jumpifov, "overflow");
	sim.connect(jumpifov, "addr[]", pc, "data[]");
	sim.connect(jumpifov, "setout", pc, "set");
	sim.connect(jumpifov, "incout", pc, "inc");

	LogicBlock& adder = createInstrAdd(sim, sim.getBoard(), n);
	adder.setPos(500 + 250 * 8, 500);
	sim.connect(instr_rom, "out[]", adder, "num[]");
	sim.connect(addressMode, "p0[]", adder, "addra[]");
	sim.connect(addressMode, "p1[]", adder, "addrb[]");
	sim.connect(addressMode, "p2[]", adder, "addro[]");
	sim.connect(registers, "out[]", adder, "memin[]");
	sim.connect(registers, "wsigout", adder, "wsigin");
	sim.connect(registers, "rsigout", adder, "rsigin");
	sim.connect(addressMode, "sigout", adder, "sigin");
	sim.connect(adder, "addr[]", registers, "addr[]");
	sim.connect(adder, "data[]", registers, "data[]");
	sim.connect(adder, "wsigout", registers, "write");
	sim.connect(adder, "rsigout", registers, "read");
	sim.connect(adder, "sigout", pc, "inc");
	sim.connect(adder, "zero", flags, "zdata");
	sim.connect(adder, "sigout", flags, "zwrite");
	sim.connect(adder, "over", flags, "odata");
	sim.connect(adder, "sigout", flags, "owrite");

	LogicBlock& subber = createInstrSub(sim, sim.getBoard(), n);
	subber.setPos(500 + 250 * 9, 500);
	sim.connect(instr_rom, "out[]", subber, "num[]");
	sim.connect(addressMode, "p0[]", subber, "addra[]");
	sim.connect(addressMode, "p1[]", subber, "addrb[]");
	sim.connect(addressMode, "p2[]", subber, "addro[]");
	sim.connect(registers, "out[]", subber, "memin[]");
	sim.connect(registers, "wsigout", subber, "wsigin");
	sim.connect(registers, "rsigout", subber, "rsigin");
	sim.connect(addressMode, "sigout", subber, "sigin");
	sim.connect(subber, "addr[]", registers, "addr[]");
	sim.connect(subber, "data[]", registers, "data[]");
	sim.connect(subber, "wsigout", registers, "write");
	sim.connect(subber, "rsigout", registers, "read");
	sim.connect(subber, "sigout", pc, "inc");
	sim.connect(subber, "zero", flags, "zdata");
	sim.connect(subber, "sigout", flags, "zwrite");
	sim.connect(subber, "over", flags, "odata");
	sim.connect(subber, "sigout", flags, "owrite");

	LogicBlock& inc = createInstrInc(sim, sim.getBoard(), n);
	inc.setPos(500 + 250 * 10, 500);
	sim.connect(instr_rom, "out[]", inc, "num[]");
	sim.connect(addressMode, "p0[]", inc, "addri[]");
	sim.connect(addressMode, "p1[]", inc, "addro[]");
	sim.connect(registers, "out[]", inc, "memin[]");
	sim.connect(registers, "wsigout", inc, "wsigin");
	sim.connect(registers, "rsigout", inc, "rsigin");
	sim.connect(addressMode, "sigout", inc, "sigin");
	sim.connect(inc, "addr[]", registers, "addr[]");
	sim.connect(inc, "data[]", registers, "data[]");
	sim.connect(inc, "wsigout", registers, "write");
	sim.connect(inc, "rsigout", registers, "read");
	sim.connect(inc, "sigout", pc, "inc");
	sim.connect(inc, "zero", flags, "zdata");
	sim.connect(inc, "sigout", flags, "zwrite");
	sim.connect(inc, "over", flags, "odata");
	sim.connect(inc, "sigout", flags, "owrite");

	LogicBlock& dec = createInstrDec(sim, sim.getBoard(), n);
	dec.setPos(500 + 250 * 11, 500);
	sim.connect(instr_rom, "out[]", dec, "num[]");
	sim.connect(addressMode, "p0[]", dec, "addri[]");
	sim.connect(addressMode, "p1[]", dec, "addro[]");
	sim.connect(registers, "out[]", dec, "memin[]");
	sim.connect(registers, "wsigout", dec, "wsigin");
	sim.connect(registers, "rsigout", dec, "rsigin");
	sim.connect(addressMode, "sigout", dec, "sigin");
	sim.connect(dec, "addr[]", registers, "addr[]");
	sim.connect(dec, "data[]", registers, "data[]");
	sim.connect(dec, "wsigout", registers, "write");
	sim.connect(dec, "rsigout", registers, "read");
	sim.connect(dec, "sigout", pc, "inc");
	sim.connect(dec, "zero", flags, "zdata");
	sim.connect(dec, "sigout", flags, "zwrite");
	sim.connect(dec, "over", flags, "odata");
	sim.connect(dec, "sigout", flags, "owrite");

	LogicBlock& multiplier = createInstrMul(sim, sim.getBoard(), n);
	multiplier.setPos(500 + 250 * 12, 500);
	sim.connect(instr_rom, "out[]", multiplier, "num[]");
	sim.connect(addressMode, "p0[]", multiplier, "addra[]");
	sim.connect(addressMode, "p1[]", multiplier, "addrb[]");
	sim.connect(addressMode, "p2[]", multiplier, "addro[]");
	sim.connect(registers, "out[]", multiplier, "memin[]");
	sim.connect(registers, "wsigout", multiplier, "wsigin");
	sim.connect(registers, "rsigout", multiplier, "rsigin");
	sim.connect(addressMode, "sigout", multiplier, "sigin");
	sim.connect(multiplier, "addr[]", registers, "addr[]");
	sim.connect(multiplier, "data[]", registers, "data[]");
	sim.connect(multiplier, "wsigout", registers, "write");
	sim.connect(multiplier, "rsigout", registers, "read");
	sim.connect(multiplier, "sigout", pc, "inc");
	sim.connect(multiplier, "zero", flags, "zdata");
	sim.connect(multiplier, "sigout", flags, "zwrite");

	LogicBlock& negate = createInstrNegate(sim, sim.getBoard(), n);
	negate.setPos(500 + 250 * 13, 500);
	sim.connect(instr_rom, "out[]", negate, "num[]");
	sim.connect(addressMode, "p0[]", negate, "addri[]");
	sim.connect(addressMode, "p1[]", negate, "addro[]");
	sim.connect(registers, "out[]", negate, "memin[]");
	sim.connect(registers, "wsigout", negate, "wsigin");
	sim.connect(registers, "rsigout", negate, "rsigin");
	sim.connect(addressMode, "sigout", negate, "sigin");
	sim.connect(negate, "addr[]", registers, "addr[]");
	sim.connect(negate, "data[]", registers, "data[]");
	sim.connect(negate, "wsigout", registers, "write");
	sim.connect(negate, "rsigout", registers, "read");
	sim.connect(negate, "sigout", pc, "inc");
	sim.connect(negate, "zero", flags, "zdata");
	sim.connect(negate, "sigout", flags, "zwrite");

	LogicBlock& lshift = createInstrLShift(sim, sim.getBoard(), n);
	lshift.setPos(500 + 250 * 14, 500);
	sim.connect(instr_rom, "out[]", lshift, "num[]");
	sim.connect(addressMode, "p0[]", lshift, "addri[]");
	sim.connect(addressMode, "p1[]", lshift, "addro[]");
	sim.connect(registers, "out[]", lshift, "memin[]");
	sim.connect(registers, "wsigout", lshift, "wsigin");
	sim.connect(registers, "rsigout", lshift, "rsigin");
	sim.connect(addressMode, "sigout", lshift, "sigin");
	sim.connect(lshift, "addr[]", registers, "addr[]");
	sim.connect(lshift, "data[]", registers, "data[]");
	sim.connect(lshift, "wsigout", registers, "write");
	sim.connect(lshift, "rsigout", registers, "read");
	sim.connect(lshift, "sigout", pc, "inc");
	sim.connect(lshift, "zero", flags, "zdata");
	sim.connect(lshift, "sigout", flags, "zwrite");

	LogicBlock& rshift = createInstrRShift(sim, sim.getBoard(), n);
	rshift.setPos(500 + 250 * 15, 500);
	sim.connect(instr_rom, "out[]", rshift, "num[]");
	sim.connect(addressMode, "p0[]", rshift, "addri[]");
	sim.connect(addressMode, "p1[]", rshift, "addro[]");
	sim.connect(registers, "out[]", rshift, "memin[]");
	sim.connect(registers, "wsigout", rshift, "wsigin");
	sim.connect(registers, "rsigout", rshift, "rsigin");
	sim.connect(addressMode, "sigout", rshift, "sigin");
	sim.connect(rshift, "addr[]", registers, "addr[]");
	sim.connect(rshift, "data[]", registers, "data[]");
	sim.connect(rshift, "wsigout", registers, "write");
	sim.connect(rshift, "rsigout", registers, "read");
	sim.connect(rshift, "sigout", pc, "inc");
	sim.connect(rshift, "zero", flags, "zdata");
	sim.connect(rshift, "sigout", flags, "zwrite");

	LogicBlock& rshiftsigned = createInstrRShiftSigned(sim, sim.getBoard(), n);
	rshiftsigned.setPos(500 + 250 * 16, 500);
	sim.connect(instr_rom, "out[]", rshiftsigned, "num[]");
	sim.connect(addressMode, "p0[]", rshiftsigned, "addri[]");
	sim.connect(addressMode, "p1[]", rshiftsigned, "addro[]");
	sim.connect(registers, "out[]", rshiftsigned, "memin[]");
	sim.connect(registers, "wsigout", rshiftsigned, "wsigin");
	sim.connect(registers, "rsigout", rshiftsigned, "rsigin");
	sim.connect(addressMode, "sigout", rshiftsigned, "sigin");
	sim.connect(rshiftsigned, "addr[]", registers, "addr[]");
	sim.connect(rshiftsigned, "data[]", registers, "data[]");
	sim.connect(rshiftsigned, "wsigout", registers, "write");
	sim.connect(rshiftsigned, "rsigout", registers, "read");
	sim.connect(rshiftsigned, "sigout", pc, "inc");
	sim.connect(rshiftsigned, "zero", flags, "zdata");
	sim.connect(rshiftsigned, "sigout", flags, "zwrite");

	LogicBlock& lcycle = createInstrLCycle(sim, sim.getBoard(), n);
	lcycle.setPos(500 + 250 * 16, 500);
	sim.connect(instr_rom, "out[]", lcycle, "num[]");
	sim.connect(addressMode, "p0[]", lcycle, "addri[]");
	sim.connect(addressMode, "p1[]", lcycle, "addro[]");
	sim.connect(registers, "out[]", lcycle, "memin[]");
	sim.connect(registers, "wsigout", lcycle, "wsigin");
	sim.connect(registers, "rsigout", lcycle, "rsigin");
	sim.connect(addressMode, "sigout", lcycle, "sigin");
	sim.connect(lcycle, "addr[]", registers, "addr[]");
	sim.connect(lcycle, "data[]", registers, "data[]");
	sim.connect(lcycle, "wsigout", registers, "write");
	sim.connect(lcycle, "rsigout", registers, "read");
	sim.connect(lcycle, "sigout", pc, "inc");
	sim.connect(lcycle, "zero", flags, "zdata");
	sim.connect(lcycle, "sigout", flags, "zwrite");

	LogicBlock& rcycle = createInstrRCycle(sim, sim.getBoard(), n);
	rcycle.setPos(500 + 250 * 16, 500);
	sim.connect(instr_rom, "out[]", rcycle, "num[]");
	sim.connect(addressMode, "p0[]", rcycle, "addri[]");
	sim.connect(addressMode, "p1[]", rcycle, "addro[]");
	sim.connect(registers, "out[]", rcycle, "memin[]");
	sim.connect(registers, "wsigout", rcycle, "wsigin");
	sim.connect(registers, "rsigout", rcycle, "rsigin");
	sim.connect(addressMode, "sigout", rcycle, "sigin");
	sim.connect(rcycle, "addr[]", registers, "addr[]");
	sim.connect(rcycle, "data[]", registers, "data[]");
	sim.connect(rcycle, "wsigout", registers, "write");
	sim.connect(rcycle, "rsigout", registers, "read");
	sim.connect(rcycle, "sigout", pc, "inc");
	sim.connect(rcycle, "zero", flags, "zdata");
	sim.connect(rcycle, "sigout", flags, "zwrite");

	LogicBlock& ixor = createInstrXor(sim, sim.getBoard(), n);
	ixor.setPos(500 + 250 * 17, 500);
	sim.connect(instr_rom, "out[]", ixor, "num[]");
	sim.connect(addressMode, "p0[]", ixor, "addra[]");
	sim.connect(addressMode, "p1[]", ixor, "addrb[]");
	sim.connect(addressMode, "p2[]", ixor, "addro[]");
	sim.connect(registers, "out[]", ixor, "memin[]");
	sim.connect(registers, "wsigout", ixor, "wsigin");
	sim.connect(registers, "rsigout", ixor, "rsigin");
	sim.connect(addressMode, "sigout", ixor, "sigin");
	sim.connect(ixor, "addr[]", registers, "addr[]");
	sim.connect(ixor, "data[]", registers, "data[]");
	sim.connect(ixor, "wsigout", registers, "write");
	sim.connect(ixor, "rsigout", registers, "read");
	sim.connect(ixor, "sigout", pc, "inc");
	sim.connect(ixor, "zero", flags, "zdata");
	sim.connect(ixor, "sigout", flags, "zwrite");

	LogicBlock& iand = createInstrAnd(sim, sim.getBoard(), n);
	iand.setPos(500 + 250 * 18, 500);
	sim.connect(instr_rom, "out[]", iand, "num[]");
	sim.connect(addressMode, "p0[]", iand, "addra[]");
	sim.connect(addressMode, "p1[]", iand, "addrb[]");
	sim.connect(addressMode, "p2[]", iand, "addro[]");
	sim.connect(registers, "out[]", iand, "memin[]");
	sim.connect(registers, "wsigout", iand, "wsigin");
	sim.connect(registers, "rsigout", iand, "rsigin");
	sim.connect(addressMode, "sigout", iand, "sigin");
	sim.connect(iand, "addr[]", registers, "addr[]");
	sim.connect(iand, "data[]", registers, "data[]");
	sim.connect(iand, "wsigout", registers, "write");
	sim.connect(iand, "rsigout", registers, "read");
	sim.connect(iand, "sigout", pc, "inc");
	sim.connect(iand, "zero", flags, "zdata");
	sim.connect(iand, "sigout", flags, "zwrite");

	LogicBlock& ior = createInstrOr(sim, sim.getBoard(), n);
	ior.setPos(500 + 250 * 19, 500);
	sim.connect(instr_rom, "out[]", ior, "num[]");
	sim.connect(addressMode, "p0[]", ior, "addra[]");
	sim.connect(addressMode, "p1[]", ior, "addrb[]");
	sim.connect(addressMode, "p2[]", ior, "addro[]");
	sim.connect(registers, "out[]", ior, "memin[]");
	sim.connect(registers, "wsigout", ior, "wsigin");
	sim.connect(registers, "rsigout", ior, "rsigin");
	sim.connect(addressMode, "sigout", ior, "sigin");
	sim.connect(ior, "addr[]", registers, "addr[]");
	sim.connect(ior, "data[]", registers, "data[]");
	sim.connect(ior, "wsigout", registers, "write");
	sim.connect(ior, "rsigout", registers, "read");
	sim.connect(ior, "sigout", pc, "inc");
	sim.connect(ior, "zero", flags, "zdata");
	sim.connect(ior, "sigout", flags, "zwrite");

	LogicBlock& invert = createInstrInvert(sim, sim.getBoard(), n);
	invert.setPos(500 + 250 * 20, 500);
	sim.connect(instr_rom, "out[]", invert, "num[]");
	sim.connect(addressMode, "p0[]", invert, "addri[]");
	sim.connect(addressMode, "p1[]", invert, "addro[]");
	sim.connect(registers, "out[]", invert, "memin[]");
	sim.connect(registers, "wsigout", invert, "wsigin");
	sim.connect(registers, "rsigout", invert, "rsigin");
	sim.connect(addressMode, "sigout", invert, "sigin");
	sim.connect(invert, "addr[]", registers, "addr[]");
	sim.connect(invert, "data[]", registers, "data[]");
	sim.connect(invert, "wsigout", registers, "write");
	sim.connect(invert, "rsigout", registers, "read");
	sim.connect(invert, "sigout", pc, "inc");
	sim.connect(invert, "zero", flags, "zdata");
	sim.connect(invert, "sigout", flags, "zwrite");

	LogicBlock& compare = createInstrCompare(sim, sim.getBoard(), n);
	compare.setPos(500 + 250 * 21, 500);
	sim.connect(instr_rom, "out[]", compare, "num[]");
	sim.connect(addressMode, "p0[]", compare, "addra[]");
	sim.connect(addressMode, "p1[]", compare, "addrb[]");
	sim.connect(registers, "out[]", compare, "memin[]");
	sim.connect(registers, "rsigout", compare, "rsigin");
	sim.connect(addressMode, "sigout", compare, "sigin");
	sim.connect(compare, "addr[]", registers, "addr[]");
	sim.connect(compare, "rsigout", registers, "read");
	sim.connect(compare, "sigout", pc, "inc");
	sim.connect(compare, "less", flags, "ldata");
	sim.connect(compare, "sigout", flags, "lwrite");
	sim.connect(compare, "equal", flags, "edata");
	sim.connect(compare, "sigout", flags, "ewrite");
	sim.connect(compare, "greater", flags, "gdata");
	sim.connect(compare, "sigout", flags, "gwrite");

	LogicBlock& compareuns = createInstrCompareUnsigned(sim, sim.getBoard(), n);
	compareuns.setPos(500 + 250 * 22, 500);
	sim.connect(instr_rom, "out[]", compareuns, "num[]");
	sim.connect(addressMode, "p0[]", compareuns, "addra[]");
	sim.connect(addressMode, "p1[]", compareuns, "addrb[]");
	sim.connect(registers, "out[]", compareuns, "memin[]");
	sim.connect(registers, "rsigout", compareuns, "rsigin");
	sim.connect(addressMode, "sigout", compareuns, "sigin");
	sim.connect(compareuns, "addr[]", registers, "addr[]");
	sim.connect(compareuns, "rsigout", registers, "read");
	sim.connect(compareuns, "sigout", pc, "inc");
	sim.connect(compareuns, "less", flags, "ldata");
	sim.connect(compareuns, "sigout", flags, "lwrite");
	sim.connect(compareuns, "equal", flags, "edata");
	sim.connect(compareuns, "sigout", flags, "ewrite");
	sim.connect(compareuns, "greater", flags, "gdata");
	sim.connect(compareuns, "sigout", flags, "gwrite");

	LogicBlock& move = createInstrMove(sim, sim.getBoard(), n);
	move.setPos(500 + 250 * 23, 500);
	sim.connect(instr_rom, "out[]", move, "num[]");
	sim.connect(addressMode, "p0[]", move, "addrsrc[]");
	sim.connect(addressMode, "p1[]", move, "addrdst[]");
	sim.connect(addressMode, "sigout", move, "sigin");
	sim.connect(registers, "out[]", move, "memin[]");
	sim.connect(registers, "rsigout", move, "rsigin");
	sim.connect(registers, "wsigout", move, "wsigin");
	sim.connect(move, "addr[]", registers, "addr[]");
	sim.connect(move, "data[]", registers, "data[]");
	sim.connect(move, "wsigout", registers, "write");
	sim.connect(move, "rsigout", registers, "read");
	sim.connect(move, "sigout", pc, "inc");

	LogicBlock& loadimm = createInstrLoadImm(sim, sim.getBoard(), n);
	loadimm.setPos(500 + 250 * 24, 500);
	sim.connect(instr_rom, "out[]", loadimm, "num[]");
	sim.connect(addressMode, "p0[]", loadimm, "addr[]");
	sim.connect(addressMode, "p1[]", loadimm, "data[]");
	sim.connect(addressMode, "sigout", loadimm, "sigin");
	sim.connect(loadimm, "addr[]", registers, "addr[]");
	sim.connect(loadimm, "data[]", registers, "data[]");
	sim.connect(loadimm, "wsigout", registers, "write");
	sim.connect(registers, "wsigout", loadimm, "wsigin");
	sim.connect(loadimm, "sigout", pc, "inc");

	LogicBlock& clearFlags = createInstrClearFlags(sim, sim.getBoard(), n);
	clearFlags.setPos(500 + 250 * 25, 500);
	sim.connect(instr_rom, "out[]", clearFlags, "num[]");
	sim.connect(romdelay, "out", clearFlags, "sigin");
	sim.connect(loadimm, "sigout", pc, "inc");
	sim.connect(clearFlags, "sigout", flags, "lwrite");
	sim.connect(clearFlags, "sigout", flags, "ewrite");
	sim.connect(clearFlags, "sigout", flags, "gwrite");
	sim.connect(clearFlags, "sigout", flags, "zwrite");
	sim.connect(clearFlags, "sigout", flags, "owrite");

	LogicBlock& highbit = createInstrGetHighBit(sim, sim.getBoard(), n);
	highbit.setPos(500 + 250 * 26, 500);
	sim.connect(instr_rom, "out[]", highbit, "num[]");
	sim.connect(addressMode, "p0[]", highbit, "addri[]");
	sim.connect(addressMode, "p1[]", highbit, "addro[]");
	sim.connect(registers, "out[]", highbit, "memin[]");
	sim.connect(registers, "wsigout", highbit, "wsigin");
	sim.connect(registers, "rsigout", highbit, "rsigin");
	sim.connect(addressMode, "sigout", highbit, "sigin");
	sim.connect(highbit, "addr[]", registers, "addr[]");
	sim.connect(highbit, "data[]", registers, "data[]");
	sim.connect(highbit, "wsigout", registers, "write");
	sim.connect(highbit, "rsigout", registers, "read");
	sim.connect(highbit, "sigout", pc, "inc");

	LogicBlock& lowbit = createInstrGetLowBit(sim, sim.getBoard(), n);
	lowbit.setPos(500 + 250 * 27, 500);
	sim.connect(instr_rom, "out[]", lowbit, "num[]");
	sim.connect(addressMode, "p0[]", lowbit, "addri[]");
	sim.connect(addressMode, "p1[]", lowbit, "addro[]");
	sim.connect(registers, "out[]", lowbit, "memin[]");
	sim.connect(registers, "wsigout", lowbit, "wsigin");
	sim.connect(registers, "rsigout", lowbit, "rsigin");
	sim.connect(addressMode, "sigout", lowbit, "sigin");
	sim.connect(lowbit, "addr[]", registers, "addr[]");
	sim.connect(lowbit, "data[]", registers, "data[]");
	sim.connect(lowbit, "wsigout", registers, "write");
	sim.connect(lowbit, "rsigout", registers, "read");
	sim.connect(lowbit, "sigout", pc, "inc");

	LogicBlock& iszero = createInstrIsZero(sim, sim.getBoard(), n);
	iszero.setPos(500 + 250 * 28, 500);
	sim.connect(instr_rom, "out[]", iszero, "num[]");
	sim.connect(addressMode, "p0[]", iszero, "addr[]");
	sim.connect(registers, "out[]", iszero, "memin[]");
	sim.connect(registers, "rsigout", iszero, "rsigin");
	sim.connect(addressMode, "sigout", iszero, "sigin");
	sim.connect(iszero, "addr[]", registers, "addr[]");
	sim.connect(iszero, "data[]", registers, "data[]");
	sim.connect(iszero, "rsigout", registers, "read");
	sim.connect(iszero, "sigout", pc, "inc");
	sim.connect(iszero, "zero", flags, "zdata");
	sim.connect(iszero, "sigout", flags, "zwrite");

	LogicBlock& setAddrMode = createInstrSetAddrMode(sim, sim.getBoard(), n);
	setAddrMode.setPos(500 + 250 * 29, 500);
	sim.connect(instr_rom, "out[]", setAddrMode, "num[]");
	sim.connect(param1_rom, "out[]", setAddrMode, "flags[]");
	sim.connect(romdelay, "out", setAddrMode, "sigin");
	sim.connect(setAddrMode, "amdata[]", flags, "amdata[]");
	sim.connect(setAddrMode, "sigout", flags, "amwrite[0]");
	sim.connect(setAddrMode, "sigout", flags, "amwrite[1]");
	sim.connect(setAddrMode, "sigout", flags, "amwrite[2]");
	sim.connect(setAddrMode, "sigout", pc, "inc");


	sim.recalc();
	sim.sortConnections();

	int mousex = 0, mousey = 0;
	int oldmousex = 0, oldmousey = 0;
	bool click = false;
	bool oldclick = false;
	
	size_t ticks = 0;
	size_t tps = 30;
	int cappedticks = 1;
	size_t lastTime = SDL_GetTicks();
	size_t deltaTime = 0, scrollDT = 0;
	bool pause = true;
	bool running = true;
	bool left = false, right = false, up = false, down = false;
	float velx = 0, vely = 0;

	bool visableWires = true;

	while (running)
	{
		oldmousex = mousex;
		oldmousey = mousey;
		oldclick = click;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;

			case SDL_EVENT_MOUSE_MOTION:
				mousex = event.motion.x;
				mousey = event.motion.y;
				if (abs(mousex - oldmousex) < 2)
					mousex = oldmousex;
				if (abs(mousey - oldmousey) < 2)
					mousey = oldmousey;
				break;

			case SDL_EVENT_KEY_DOWN:
				switch (event.key.scancode)
				{
				case SDL_SCANCODE_DOWN: down = true; break;
				case SDL_SCANCODE_UP: up = true; break;
				case SDL_SCANCODE_LEFT: left = true; break;
				case SDL_SCANCODE_RIGHT: right = true; break;
				case SDL_SCANCODE_LEFTBRACKET: sim.zoom(-1, surface->w, surface->h); break;
				case SDL_SCANCODE_RIGHTBRACKET: sim.zoom(1, surface->w, surface->h); break;
				case SDL_SCANCODE_P: pause = !pause; break;
				case SDL_SCANCODE_RETURN: if (pause) { sim.tick(); ++ticks; } break;
				case SDL_SCANCODE_0: cappedticks = 0; break;
				case SDL_SCANCODE_1: cappedticks = 1; break;
				case SDL_SCANCODE_2: cappedticks = 5; break;
				case SDL_SCANCODE_3: cappedticks = 10; break;
				case SDL_SCANCODE_4: cappedticks = 30; break;
				case SDL_SCANCODE_5: cappedticks = 100; break;
				case SDL_SCANCODE_R: sim.reset(); ticks = 0; break;
				case SDL_SCANCODE_BACKSLASH: visableWires = !visableWires; 
					sim.setWireVisability(visableWires); break;
				}
				break;

			case SDL_EVENT_KEY_UP:
				switch (event.key.scancode)
				{
				case SDL_SCANCODE_DOWN: down = false; break;
				case SDL_SCANCODE_UP: up = false; break;
				case SDL_SCANCODE_LEFT: left = false; break;
				case SDL_SCANCODE_RIGHT: right = false; break;
				}
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN: click = true; break;
			case SDL_EVENT_MOUSE_BUTTON_UP: click = false; break;

			case SDL_EVENT_WINDOW_RESIZED:
				width = event.window.data1;
				height = event.window.data2;
				surface = SDL_GetWindowSurface(window);
				break;
			}
		}

		sim.interact(mousex, mousey, oldmousex, oldmousey, click, oldclick);

		size_t nowTime = SDL_GetTicks();
		deltaTime += nowTime - lastTime;
		scrollDT += nowTime - lastTime;
		while (scrollDT >= 1000 / 60)
		{
			scrollDT -= 1000 / 60;

			velx += (left - right) * 2.f;
			vely += (up - down) * 2.f;
			velx *= 0.8f;
			vely *= 0.8f;
			sim.scroll(velx, vely);
		}
		if (deltaTime >= 1000 / tps)
		{
			deltaTime -= 1000 / tps;
			if (cappedticks == 0)
			{
				if (deltaTime >= 1000 / tps)
					tps = std::max(tps - 1, 1ull);
				else
					++tps;
			}
			else
				tps = cappedticks;

			if (!pause)
			{
				sim.tick();
				++ticks;
			}
		}
		lastTime = nowTime;

		//if (pc.output[8].data)
		//	pause = true;

		clearScreen(surface, rgb(0, 0, 0));

		// points to center of board in case you get lost
		drawLineFromPoints(surface,
			sim.getBoard().getPos().x + sim.getBoard().getSize().x / 2,
			sim.getBoard().getPos().y + sim.getBoard().getSize().y / 2,
			surface->w / 2,
			surface->h / 2,
			rgb(255, 0, 0));
		sim.draw(surface);

		LogicBlock* held = sim.getHeld();
		if (held)
		{
			drawTextFA(surface, surface->w - 1, surface->h - 1, 1, 3, 1, rgb(255, 255, 255), "Held: %s", held->getLabel().data());
			drawTextFA(surface, surface->w - 1, surface->h - 1, 1, 1, 1, rgb(255, 255, 255), "Pos: %.1f, %.1f", held->getRelPos().x, held->getRelPos().y);
		}

		drawTextFA(surface, 0, surface->h - 1, -1, 5, 1, rgb(255, 255, 255), "Blocks: %llu", LogicBlock::blockCount);
		drawTextFA(surface, 0, surface->h - 1, -1, 7, 1, rgb(255, 255, 255), "Wires: %llu", sim.wireCount);

		drawTextFA(surface, 0, surface->h - 1, -1, 3, 1, rgb(255, 255, 255), "Ticks: %llu", ticks);
		if (pause)
			drawTextA(surface, 0, surface->h - 1, -1, 1, 1, rgb(255, 255, 255), "Paused");
		else
			drawTextFA(surface, 0, surface->h - 1, -1, 1, 1, rgb(255, 255, 255), "TPS: %llu", tps);

		SDL_UpdateWindowSurface(window);
	}
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}