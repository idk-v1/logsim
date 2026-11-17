#include "logicsim.h"
#include "customblocks.h"

#include <iostream>

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

	int64_t instr_data[]  = { 0x23, 0x23, 0x17, 0x05, 0x08, 0x0A, 0x02, 0x01 };
	int64_t param1_data[] = { 0x00, 0x02, 0x00, 0x07, 0x00, 0x00, 0x02, 0x00 };
	int64_t param2_data[] = { 0x01, 0x0A, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00 };
	int64_t param3_data[] = { 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };
	LogicBlock& instr_rom  = createAddrROM(sim, sim.getBoard(), n, n, sizeof(instr_data) / sizeof(*instr_data), instr_data);
	LogicBlock& param1_rom = createAddrROM(sim, sim.getBoard(), n, n, sizeof(instr_data) / sizeof(*instr_data), param1_data);
	LogicBlock& param2_rom = createAddrROM(sim, sim.getBoard(), n, n, sizeof(instr_data) / sizeof(*instr_data), param2_data);
	LogicBlock& param3_rom = createAddrROM(sim, sim.getBoard(), n, n, sizeof(instr_data) / sizeof(*instr_data), param3_data);

	LogicBlock& registers = createAddrMemory(sim, sim.getBoard(), n, n, 3);
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

	LogicBlock& read = sim.add(LogicBlockToggle);
	read.setPos(250, 150);
	sim.connect(read, "out", instr_rom, "read");
	sim.connect(read, "out", param1_rom, "read");
	sim.connect(read, "out", param2_rom, "read");
	sim.connect(read, "out", param3_rom, "read");

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

	

	LogicBlock& loadimm = createInstrLoadImm(sim, sim.getBoard(), n);
	loadimm.setPos(500, 500);
	sim.connect(instr_rom, "out[]", loadimm, "num[]");
	sim.connect(param1_rom, "out[]", loadimm, "addr[]");
	sim.connect(param2_rom, "out[]", loadimm, "data[]");
	sim.connect(romdelay, "out", loadimm, "sigin");
	sim.connect(loadimm, "addr[]", registers, "addr[]");
	sim.connect(loadimm, "data[]", registers, "data[]");
	sim.connect(loadimm, "wsigout", registers, "write");
	sim.connect(registers, "wsigout", loadimm, "wsigin");
	sim.connect(loadimm, "sigout", pc, "inc");

	LogicBlock& move = createInstrMove(sim, sim.getBoard(), n);
	move.setPos(750, 500);
	sim.connect(instr_rom, "out[]", move, "num[]");
	sim.connect(param1_rom, "out[]", move, "addrsrc[]");
	sim.connect(param2_rom, "out[]", move, "addrdst[]");
	sim.connect(romdelay, "out", move, "sigin");
	sim.connect(registers, "out[]", move, "memin[]");
	sim.connect(registers, "rsigout", move, "rsigin");
	sim.connect(registers, "wsigout", move, "wsigin");
	sim.connect(move, "addr[]", registers, "addr[]");
	sim.connect(move, "data[]", registers, "data[]");
	sim.connect(move, "wsigout", registers, "write");
	sim.connect(move, "rsigout", registers, "read");
	sim.connect(move, "sigout", pc, "inc");

	LogicBlock& compare = createInstrCompare(sim, sim.getBoard(), n);
	compare.setPos(1000, 500);
	sim.connect(instr_rom, "out[]", compare, "num[]");
	sim.connect(param1_rom, "out[]", compare, "addra[]");
	sim.connect(param2_rom, "out[]", compare, "addrb[]");
	sim.connect(registers, "out[]", compare, "memin[]");
	sim.connect(registers, "rsigout", compare, "rsigin");
	sim.connect(romdelay, "out", compare, "sigin");
	sim.connect(compare, "addr[]", registers, "addr[]");
	sim.connect(compare, "rsigout", registers, "read");
	sim.connect(compare, "sigout", pc, "inc");
	sim.connect(compare, "less", flags, "ldata");
	sim.connect(compare, "sigout", flags, "lwrite");
	sim.connect(compare, "equal", flags, "edata");
	sim.connect(compare, "sigout", flags, "ewrite");
	sim.connect(compare, "greater", flags, "gdata");
	sim.connect(compare, "sigout", flags, "gwrite");


	LogicBlock& jumpifgr = createInstrJumpGr(sim, sim.getBoard(), n);
	jumpifgr.setPos(1250, 500);
	sim.connect(instr_rom, "out[]", jumpifgr, "num[]");
	sim.connect(param1_rom, "out[]", jumpifgr, "addr[]");
	sim.connect(romdelay, "out", jumpifgr, "sigin");
	sim.connect(flags, "greater", jumpifgr, "greater");
	sim.connect(jumpifgr, "addr[]", pc, "data[]");
	sim.connect(jumpifgr, "setout", pc, "set");
	sim.connect(jumpifgr, "incout", pc, "inc");

	LogicBlock& jump = createInstrJump(sim, sim.getBoard(), n);
	jump.setPos(1500, 500);
	sim.connect(instr_rom, "out[]", jump, "num[]");
	sim.connect(param1_rom, "out[]", jump, "addr[]");
	sim.connect(romdelay, "out", jump, "sigin");
	sim.connect(jump, "addr[]", pc, "data[]");
	sim.connect(jump, "setout", pc, "set");

	LogicBlock& adder = createInstrAdd(sim, sim.getBoard(), n);
	adder.setPos(1750, 500);
	sim.connect(instr_rom, "out[]", adder, "num[]");
	sim.connect(param1_rom, "out[]", adder, "addra[]");
	sim.connect(param2_rom, "out[]", adder, "addrb[]");
	sim.connect(param3_rom, "out[]", adder, "addro[]");
	sim.connect(registers, "out[]", adder, "memin[]");
	sim.connect(registers, "wsigout", adder, "wsigin");
	sim.connect(registers, "rsigout", adder, "rsigin");
	sim.connect(romdelay, "out", adder, "sigin");
	sim.connect(adder, "addr[]", registers, "addr[]");
	sim.connect(adder, "data[]", registers, "data[]");
	sim.connect(adder, "wsigout", registers, "write");
	sim.connect(adder, "rsigout", registers, "read");
	sim.connect(adder, "sigout", pc, "inc");
	sim.connect(adder, "zero", flags, "zdata");
	sim.connect(adder, "sigout", flags, "zwrite");
	sim.connect(adder, "over", flags, "odata");
	sim.connect(adder, "sigout", flags, "owrite");

	LogicBlock& inc = createInstrInc(sim, sim.getBoard(), n);
	inc.setPos(2000, 500);
	sim.connect(instr_rom, "out[]", inc, "num[]");
	sim.connect(param1_rom, "out[]", inc, "addr[]");
	sim.connect(registers, "out[]", inc, "memin[]");
	sim.connect(registers, "wsigout", inc, "wsigin");
	sim.connect(registers, "rsigout", inc, "rsigin");
	sim.connect(romdelay, "out", inc, "sigin");
	sim.connect(inc, "addr[]", registers, "addr[]");
	sim.connect(inc, "data[]", registers, "data[]");
	sim.connect(inc, "wsigout", registers, "write");
	sim.connect(inc, "rsigout", registers, "read");
	sim.connect(inc, "sigout", pc, "inc");
	sim.connect(inc, "zero", flags, "zdata");
	sim.connect(inc, "sigout", flags, "zwrite");
	sim.connect(inc, "over", flags, "odata");
	sim.connect(inc, "sigout", flags, "owrite");



	sim.recalc();
	sim.sortConnections();

	int mousex = 0, mousey = 0;
	int oldmousex = 0, oldmousey = 0;
	bool click = false;
	bool oldclick = false;
	
	size_t ticks = 0;
	size_t tps = 30;
	size_t lastTime = SDL_GetTicks();
	size_t deltaTime = 0, scrollDT = 0;
	bool pause = true;
	bool running = true;
	bool left = false, right = false, up = false, down = false;
	float velx = 0, vely = 0;
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
			if (deltaTime >= 1000 / tps)
				tps = std::max(tps - 1, 1ull);
			else
				++tps;

			if (!pause)
			{
				sim.tick();
				++ticks;
			}
		}
		lastTime = nowTime;

		sim.recalc();

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
			drawTextFA(surface, 0, surface->h - 1, -1, 3, 1, rgb(255, 255, 255), "Held: %s", held->label.data());
			drawTextFA(surface, 0, surface->h - 1, -1, 1, 1, rgb(255, 255, 255), "Pos: %d, %d", held->getRelPos().x, held->getRelPos().y);
		}

		drawTextFA(surface, surface->w - 1, surface->h - 1, 1, 3, 1, rgb(255, 255, 255), "Ticks: %llu", ticks);
		if (pause)
			drawTextA(surface, surface->w - 1, surface->h - 1, 1, 1, 1, rgb(255, 255, 255), "Paused");
		else
			drawTextFA(surface, surface->w - 1, surface->h - 1, 1, 1, 1, rgb(255, 255, 255), "TPS: %llu", tps);

		SDL_UpdateWindowSurface(window);
	}
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}