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

	int stepTime = 0;

	LogicSim sim;

	int64_t data[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	LogicBlock& rom = createAddrROM(sim, sim.getBoard(), 8, 8, 8, data);
	LogicBlock& edit = createNBitNumberEditorCtrl(sim, sim.getBoard(), 8);
	LogicBlock& read = sim.add(LogicBlockToggle);
	rom.setPos(300, 15);
	read.setPos(0, 300);
	sim.connect(edit, "out[]", rom, "addr[]");
	sim.connect(read, "out", rom, "read");

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



		drawTextFA(surface, 0, 600, -1, 1, 1, rgb(255, 255, 255), "Ticks: %d", ticks);

		SDL_UpdateWindowSurface(window);

		if (!pause)
		{
			sim.tick(mousex, mousey, click);
			++ticks;
		}

		SDL_Delay(stepTime);
	}
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}