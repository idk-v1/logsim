#include "logicsim.h"
#include "customblocks.h"

#include <iostream>

int main()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("logsim", 1200, 600, 0);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	setPixelFormat(surface);

	int stepTime = 500;

	LogicSim sim;

	//LogicBlock& clock1 = createClockN(sim, sim.getBoard(), 1);
	//clock1.setPos(5, 15);
	//LogicBlock& clock2 = createClockN(sim, sim.getBoard(), 2);
	//clock2.setPos(115, 15);
	//LogicBlock& clock3 = createClockN(sim, sim.getBoard()), 3);
	//clock3.setPos(225, 15);
	//LogicBlock& power = sim.add(LogicBlockOn);
	//sim.connect(power, "out", clock1, "pause");

	//LogicBlock& clock = createClockN(sim, sim.getBoard(), 7);
	//clock.setPos(15, 15);
	//LogicBlock& pulse = createPulse(sim, sim.getBoard());
	//pulse.setPos(125, 15);
	//
	//sim.connect(clock, "out", pulse, "in");

	int n = 16;
	LogicBlock& a = createNBitNumber(sim, sim.getBoard(), n, 10);
	LogicBlock& b = createNBitNumber(sim, sim.getBoard(), n, 1);
	LogicBlock& sub = createNBitSub(sim, sim.getBoard(), n);
	LogicBlock& out = createNBitOutput(sim, sim.getBoard(), n);
	LogicBlock& power = sim.add(LogicBlockOn);
	a.setPos(15, 50);
	b.setPos(15, 150);
	sub.setPos(100, 50);
	out.setPos(1000, 50);
	power.setPos(15, 350);
	
	sim.connect(a, "out[]", sub, "a[]");
	sim.connect(b, "out[]", sub, "b[]");
	
	sim.connect(power, "out", sub, "sigin");
	
	LogicBlock& shift = createNBitShiftM(sim, sim.getBoard(), n, 1);
	sim.connect(sub, "out[]", shift, "in[]");
	sim.connect(shift, "out[]", out, "in[]");
	shift.setPos(800, 50);

	LogicBlock& iszero = createNBitIsZero(sim, sim.getBoard(), n);
	sim.connect(sub, "out[]", iszero, "in[]");
	iszero.setPos(1000, 250);

	sim.recalc();
	
	int ticks = 0;
	bool pause = false;
	bool running = true;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;
			}
		}
		clearScreen(surface, rgb(0, 0, 0));

		sim.draw(surface);

		drawNumberLabel(surface, a, n, false);
		drawNumberLabel(surface, b, n, false);
		drawNumberLabel(surface, out, n, true);
		
		drawTextFA(surface, 0, 600, -1, 1, 1, rgb(255, 255, 255), "Ticks: %d", ticks);

		SDL_UpdateWindowSurface(window);

		if (!pause)
		{
			sim.tick();
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