#include "logicsim.h"
#include "customblocks.h"

#include <iostream>

int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("logsim", 1200, 600, 0);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	setPixelFormat(surface);

	int stepTime = 50;

	LogicSim sim;

	//LogicBlock& clock1 = createClockN(sim, sim.getBoard(), 1);
	//clock1.setPos(75, 5);
	//LogicBlock& power = sim.add(LogicBlockToggle);
	//power.setPos(5, 5);
	//sim.connect(power, "out", clock1, "pause");

	LogicBlock& input00 = createNBitNumber(sim, sim.getBoard(), 8, 127);
	LogicBlock& input01 = sim.add(LogicBlockToggle);
	LogicBlock& input02 = createNBitGate(sim, sim.getBoard(), 8);
	sim.connect(input00, "out[]", input02, "in[]");
	sim.connect(input01, "out", input02, "write");
	input00.setPos(15, 15);
	input01.setPos(15, 75);
	input02.setPos(100, 15);
	input02.setExpand(false);

	LogicBlock& input10 = createNBitNumber(sim, sim.getBoard(), 8, 34);
	LogicBlock& input11 = sim.add(LogicBlockToggle);
	LogicBlock& input12 = createNBitGate(sim, sim.getBoard(), 8);
	sim.connect(input10, "out[]", input12, "in[]");
	sim.connect(input11, "out", input12, "write");
	input10.setPos(15, 115);
	input11.setPos(15, 175);
	input12.setPos(100, 115);
	input12.setExpand(false);

	LogicBlock& input20 = createNBitNumber(sim, sim.getBoard(), 8, 213);
	LogicBlock& input21 = sim.add(LogicBlockToggle);
	LogicBlock& input22 = createNBitGate(sim, sim.getBoard(), 8);
	sim.connect(input20, "out[]", input22, "in[]");
	sim.connect(input21, "out", input22, "write");
	input20.setPos(15, 215);
	input21.setPos(15, 275);
	input22.setPos(100, 215);
	input22.setExpand(false);

	LogicBlock& mem = createNBitMemory(sim, sim.getBoard(), 8);
	mem.setPos(250, 15);
	sim.connect(input02, "out[]", mem, "data[]");
	sim.connect(input01, "out", mem, "write");
	sim.connect(input12, "out[]", mem, "data[]");
	sim.connect(input11, "out", mem, "write");
	sim.connect(input22, "out[]", mem, "data[]");
	sim.connect(input21, "out", mem, "write");

	LogicBlock& out = createNBitOutput(sim, sim.getBoard(), 8);
	out.setPos(400, 15);
	sim.connect(mem, "out[]", out, "in[]");


	//LogicBlock& clock = createClockN(sim, sim.getBoard(), 7);
	//clock.setPos(15, 15);
	//LogicBlock& pulse = createPulse(sim, sim.getBoard());
	//pulse.setPos(125, 15);
	//sim.connect(clock, "out", pulse, "in");

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
			}
		}
		clearScreen(surface, rgb(0, 0, 0));

		sim.draw(surface);

		drawNumberLabel(surface, input00, 8, false);
		drawNumberLabel(surface, input10, 8, false);
		drawNumberLabel(surface, input20, 8, false);
		drawNumberLabel(surface, out, 8, true);
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
		//if (sub.output["sigout"])
		//	pause = true;
		SDL_Delay(stepTime);
	}
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}