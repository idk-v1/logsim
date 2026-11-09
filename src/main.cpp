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

	//int64_t data[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	//LogicBlock& rom = createAddrROM(sim, sim.getBoard(), 8, 8, 8, data);
	LogicBlock& edit0 = createNBitNumberEditorCtrl(sim, sim.getBoard(), 8);
	edit0.setPos(0, 0);
	LogicBlock& edit1 = createNBitNumberEditorCtrl(sim, sim.getBoard(), 8);
	edit1.setPos(0, 250);
	LogicBlock& comp = createNBitCompare(sim, sim.getBoard(), 8);
	comp.setPos(250, 0);
	sim.connect(edit0, "out[]", comp, "a[]");
	sim.connect(edit1, "out[]", comp, "b[]");
	LogicBlock& signal = sim.add(LogicBlockToggle);
	sim.connect(signal, "out", comp, "sigin");
	signal.setPos(0, 500);
	//LogicBlock& read = sim.add(LogicBlockToggle);
	//rom.setPos(300, 15);
	//read.setPos(0, 300);
	//sim.connect(edit, "out[]", rom, "addr[]");
	//sim.connect(read, "out", rom, "read");

	sim.recalc();
	//sim.setZoom(1, surface->w, surface->h);

	int mousex = 0, mousey = 0;
	int oldmousex = 0, oldmousey = 0;
	bool click = false;
	bool oldclick = false;
	
	int ticks = 0;
	bool pause = false;
	bool running = true;
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
				case SDL_SCANCODE_DOWN:
					sim.scroll(0, -10);
					break;
				case SDL_SCANCODE_UP:
					sim.scroll(0, 10);
					break;
				case SDL_SCANCODE_LEFT:
					sim.scroll(10, 0);
					break;
				case SDL_SCANCODE_RIGHT:
					sim.scroll(-10, 0);
					break;
				case SDL_SCANCODE_LEFTBRACKET:
					sim.zoom(-1, surface->w, surface->h);
					break;
				case SDL_SCANCODE_RIGHTBRACKET:
					sim.zoom(1, surface->w, surface->h);
					break;
				}
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				click = true;
				break;

			case SDL_EVENT_MOUSE_BUTTON_UP:
				click = false;
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
			sim.interact(mousex, mousey, oldmousex, oldmousey, click, oldclick);
			sim.tick();
			sim.recalc();
			++ticks;
		}

		SDL_Delay(stepTime);
	}
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}