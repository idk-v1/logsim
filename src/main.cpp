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

	int stepTime = 10;

	LogicSim sim;
	int n = 8;

	LogicBlock& startbtn = sim.add(LogicBlockButton);

	int64_t instr_data[]  = { 0x22, 0x22, 0x1F, 0x1F, 0x17, 0x05, 0x1F, 0x08, 0x1F, 0x0A, 0x02, 0x01 };
	int64_t param1_data[] = { 0x02, 0x05, 0x02, 0x05, 0x00, 0x0B, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00 };
	int64_t param2_data[] = { 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00 };
	LogicBlock& instr_rom  = createAddrROM(sim, sim.getBoard(), n, n, sizeof(instr_data) / sizeof(*instr_data), instr_data);
	LogicBlock& param1_rom = createAddrROM(sim, sim.getBoard(), n, n, sizeof(instr_data) / sizeof(*instr_data), param1_data);
	LogicBlock& param2_rom = createAddrROM(sim, sim.getBoard(), n, n, sizeof(instr_data) / sizeof(*instr_data), param2_data);

	LogicBlock& pc = createProgramCounter(sim, sim.getBoard(), n);
	LogicBlock& incbtn = sim.add(LogicBlockButton);
	sim.connect(incbtn, "out", pc, "inc");
	LogicBlock& edit = createNBitNumberEditorCtrl(sim, sim.getBoard(), n);
	LogicBlock& setbtn = sim.add(LogicBlockButton);
	sim.connect(setbtn, "out", pc, "set");
	sim.connect(edit, "out[]", pc, "data[]");



	sim.recalc();

	int mousex = 0, mousey = 0;
	int oldmousex = 0, oldmousey = 0;
	bool click = false;
	bool oldclick = false;
	
	size_t ticks = 0;
	bool pause = false;
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

		velx += (left - right) * 2.f;
		vely += (up - down) * 2.f;
		velx *= 0.7f;
		vely *= 0.7f;
		sim.scroll(velx, vely);

		clearScreen(surface, rgb(0, 0, 0));

		// points to center of board in case you get lost
		drawLineFromPoints(surface,
			sim.getBoard().getPos().x + sim.getBoard().getSize().x / 2,
			sim.getBoard().getPos().y + sim.getBoard().getSize().y / 2,
			surface->w / 2,
			surface->h / 2,
			rgb(255, 0, 0));
		sim.draw(surface);



		drawTextFA(surface, 0, 600, -1, 1, 1, rgb(255, 255, 255), "Ticks: %llu", ticks);

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