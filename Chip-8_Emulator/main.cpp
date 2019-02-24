#include <iostream>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <SDL.h>

#include "chip8.h"


chip8 myChip8;

// Display size
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;
// Window size
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 320;

bool init();
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;

uint8_t keymap[16] = {
	SDLK_x,
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_q,
	SDLK_w,
	SDLK_e,
	SDLK_a,
	SDLK_s,
	SDLK_d,
	SDLK_z,
	SDLK_c,
	SDLK_4,
	SDLK_r,
	SDLK_f,
	SDLK_v,
};


bool init()
{
	window = NULL;
	renderer = NULL;
	texture = NULL;
	bool success = true;
	myChip8 = chip8();
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL could not initialize everything, SDL_ERROR: %s\n",
			SDL_GetError());
		success = false;
	}
	else {
		window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT,
			SDL_WINDOW_SHOWN);
		if (window == NULL) {
			printf("Window could not be created, SDL_ERROR: %s\n",
				SDL_GetError());
			success = false;
		}

		renderer = SDL_CreateRenderer(window, -1, 0);
		SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
			SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	}
	return success;
}



int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Usage: myChip8.exe chip8application\n\n");
		return 1;
	}
	if (!init()) {
		return 0;
	}
	
	if (!myChip8.loadApplication(argv[1]))
		return 0;
	printf("Everything has been initialized and the ROM loaded\n");
	uint32_t buff[2048];
	bool quit = false;
	while (true) {

		myChip8.emulateCycle();

		SDL_Event e;
		while (SDL_PollEvent(&e)) {

			if (e.type == SDL_QUIT)
				exit(0);
			
			if(e.type == SDL_KEYDOWN) {
				for (int i = 0; i < 16; i++) {
					if (e.key.keysym.sym == keymap[i])
						myChip8.keypad[i] = 1;
				}
			}
			if (e.type == SDL_KEYUP) {
				for (int i = 0; i < 16; i++) {
					if (e.key.keysym.sym == keymap[i])
						myChip8.keypad[i] = 0;
				}
			}
			
		}
		if (myChip8.drawFlag) {
			myChip8.drawFlag = false;

			for (int i = 0; i < 2048; i++) {
				uint8_t pixel = myChip8.graphics[i];
				buff[i] = (0x00FFFFFF * pixel) | 0xFF000000;
			}
			SDL_UpdateTexture(texture, NULL, buff, 64 * sizeof(Uint32));
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	

}
