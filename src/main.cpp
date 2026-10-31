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

	int stepTime = 100;

	LogicSim sim;

	LogicBlock& mem = createMemoryAddr(sim, sim.getBoard(), 8, 8, 2);
	mem.setPos(150, 15);

	LogicBlock& addr = createNBitNumber(sim, sim.getBoard(), 8, 1);
	addr.setPos(15, 15);
	LogicBlock& data = createNBitNumber(sim, sim.getBoard(), 8, 127);
	data.setPos(15, 75);
	LogicBlock& button = sim.add(LogicBlockToggle);
	button.setPos(15, 150);
	LogicBlock& addrgate = createNBitGate(sim, sim.getBoard(), 8);
	addrgate.setPos(50, 15);
	addrgate.setExpand(false);
	LogicBlock& datagate = createNBitGate(sim, sim.getBoard(), 8);
	datagate.setPos(50, 75);
	datagate.setExpand(false);
	LogicBlock& read = sim.add(LogicBlockToggle);
	read.setPos(15, 175);
	LogicBlock& write = sim.add(LogicBlockToggle);
	write.setPos(15, 200);

	sim.connect(addr, "out[]", addrgate, "in[]");
	sim.connect(data, "out[]", datagate, "in[]");
	sim.connect(button, "out", addrgate, "write");
	sim.connect(button, "out", datagate, "write");
	sim.connect(addrgate, "out[]", mem, "addr[]");
	sim.connect(datagate, "out[]", mem, "data[]");
	sim.connect(write, "out", mem, "write");
	sim.connect(read, "out", mem, "read");

	// Add 1 loop
	//int n = 16;
	//LogicBlock& add = createNBitAdd1(sim, sim.getBoard(), n);
	//add.setPos(100, 250);
	//add.setExpand(false);
	//
	//LogicBlock& mem = createNBitMemory(sim, sim.getBoard(), n);
	//mem.setPos(100, 15);
	//mem.setExpand(false);
	//
	//LogicBlock& button = sim.add(LogicBlockButton);
	//button.setPos(15, 15);
	//LogicBlock& toggle = sim.add(LogicBlockToggle);
	//toggle.setPos(15, 50);
	//
	//sim.connect(mem, "out[]", add, "a[]");
	//sim.connect(add, "out[]", mem, "data[]");
	//sim.connect(add, "sigout", mem, "write");
	//
	//sim.connect(mem, "sigout", add, "sigin");
	//sim.connect(button, "out", add, "sigin");
	//sim.connect(toggle, "out", mem, "read");


	// Subtract
	//int n = 32;
	//LogicBlock& a = createNBitNumber(sim, sim.getBoard(), n, 54321);
	//LogicBlock& b = createNBitNumber(sim, sim.getBoard(), n, 12345);
	//LogicBlock& sub = createNBitSub(sim, sim.getBoard(), n);
	//LogicBlock& out = createNBitOutput(sim, sim.getBoard(), n);
	//LogicBlock& power = sim.add(LogicBlockOn);
	//a.setPos(15, 50);
	//b.setPos(15, 150);
	//sub.setPos(100, 50);
	//out.setPos(1000, 50);
	//power.setPos(15, 350);
	//
	//sim.connect(a, "out[]", sub, "a[]");
	//sim.connect(b, "out[]", sub, "b[]");
	//
	//sim.connect(power, "out", sub, "sigin");
	//
	//LogicBlock& shift = createNBitShiftM(sim, sim.getBoard(), n, 1);
	//sim.connect(sub, "out[]", shift, "in[]");
	//sim.connect(shift, "out[]", out, "in[]");
	//shift.setPos(800, 50);
	//
	//LogicBlock& iszero = createNBitIsZero(sim, sim.getBoard(), n);
	//sim.connect(sub, "out[]", iszero, "in[]");
	//iszero.setPos(1000, 250);

	sim.recalc();

	int mousex = 0, mousey = 0;
	bool click = false;
	
	int ticks = 0;
	bool pause = false;
	bool running = true;
	while (running)
	{
		click = false;

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
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				click = true;
				break;

			case SDL_EVENT_WINDOW_RESIZED:
				width = event.window.data1;
				height = event.window.data2;
				surface = SDL_GetWindowSurface(window);
				break;
			}
		}
		clearScreen(surface, rgb(0, 0, 0));

		sim.draw(surface);

		// Add 1 loop
		//drawNumberLabel(surface, mem, n);
		// Subtract
		//drawNumberLabel(surface, a, n);
		//drawNumberLabel(surface, b, n);
		//drawNumberLabel(surface, out, n);
		
		drawTextFA(surface, 0, 600, -1, 1, 1, rgb(255, 255, 255), "Ticks: %d", ticks);

		SDL_UpdateWindowSurface(window);

		if (!pause)
		{
			sim.tick(mousex, mousey, click);
			++ticks;
		}
		//if (mem.output["sigout"].data)
		//	pause = true;
		SDL_Delay(stepTime);
	}
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}