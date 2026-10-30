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

	int stepTime = 5;

	LogicSim sim;

	int n = 16;

	LogicBlock& add = createNBitAdd1(sim, sim.getBoard(), n);
	add.setPos(100, 250);
	add.setExpand(false);

	LogicBlock& mem = createNBitMemory(sim, sim.getBoard(), n);
	mem.setPos(100, 15);
	mem.setExpand(false);

	LogicBlock& button = sim.add(LogicBlockButton);
	button.setPos(15, 15);
	LogicBlock& toggle = sim.add(LogicBlockToggle);
	toggle.setPos(15, 50);

	sim.connect(mem, "out[]", add, "a[]");
	sim.connect(add, "out[]", mem, "data[]");
	sim.connect(add, "sigout", mem, "write");

	sim.connect(mem, "sigout", add, "sigin");
	sim.connect(button, "out", add, "sigin");
	sim.connect(toggle, "out", mem, "read");


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

		//drawNumberLabel(surface, input00, 8, false);
		//drawNumberLabel(surface, input10, 8, false);
		//drawNumberLabel(surface, input20, 8, false);
		drawNumberLabel(surface, mem, n);
		//drawNumberLabel(surface, a, n, false);
		//drawNumberLabel(surface, b, n, false);
		//drawNumberLabel(surface, out, n, true);
		
		drawTextFA(surface, 0, 600, -1, 1, 1, rgb(255, 255, 255), "Ticks: %d", ticks);

		SDL_UpdateWindowSurface(window);

		if (!pause)
		{
			sim.tick(mousex, mousey, click);
			++ticks;
		}
		//if (mem.output["sigout"].data)
		//	pause = true;
		//SDL_Delay(stepTime);
	}
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}