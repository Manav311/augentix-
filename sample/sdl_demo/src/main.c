#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#define CLIENT_WIDTH 320
#define CLIENT_HEIGHT 240

#define RECT_WIDTH 80
#define RECT_HEIGHT 53

int fillWindow(SDL_Surface* surface)
{
	Uint32 color = SDL_MapRGB(surface->format, 0xff, 0xff, 0xff);
	return SDL_FillRect(surface, NULL, color);
}

int main()
{
	SDL_Window* window = NULL;
	SDL_Surface* surface = NULL;
	SDL_Event test_event;
	short quit = 0;

	/* Initialize rectangles */
	SDL_Rect rect1 = {
		.x = 20, .y = 20,
		.w = RECT_WIDTH, .h = RECT_HEIGHT,
	};
	SDL_Rect rect2 = {
		.x = 120, .y = 20,
		.w = RECT_WIDTH, .h = RECT_HEIGHT,
	};
	SDL_Rect rect3 = {
		.x = 220, .y = 20,
		.w = RECT_WIDTH, .h = RECT_HEIGHT,
	};
	SDL_Rect rect4 = {
		.x = 20, .y = 93,
		.w = RECT_WIDTH, .h = RECT_HEIGHT,
	};
	SDL_Rect rect5 = {
		.x = 120, .y = 93,
		.w = RECT_WIDTH, .h = RECT_HEIGHT,
	};
	SDL_Rect rect6 = {
		.x = 220, .y = 93,
		.w = RECT_WIDTH, .h = RECT_HEIGHT,
	};

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	window = SDL_CreateWindow(
		"hello_sdl2",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		CLIENT_WIDTH, CLIENT_HEIGHT,
		SDL_WINDOW_BORDERLESS
	);

	if (window == NULL) {
		fprintf(stderr, "could not create window: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	surface = SDL_GetWindowSurface(window);

	fillWindow(surface);

	puts("Draw rectangles");
	Uint32 cBlack = SDL_MapRGB(surface->format, 0x00, 0x00, 0x00);
	SDL_FillRect(surface, &rect1, cBlack);
	SDL_FillRect(surface, &rect2, cBlack);
	SDL_FillRect(surface, &rect3, cBlack);
	SDL_FillRect(surface, &rect4, cBlack);
	SDL_FillRect(surface, &rect5, cBlack);

	SDL_UpdateWindowSurface(window);

	while (!quit) {
		while (SDL_PollEvent(&test_event)) {
			switch (test_event.type) {
				case SDL_MOUSEBUTTONDOWN:
					/*
					printf("We got a mouse click event.\n");
					printf("Current mouse position is: (%d, %d).\n", test_event.button.x, test_event.button.y);
					*/
					SDL_FillRect(surface, &rect6, cBlack);
					SDL_UpdateWindowSurface(window);
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				default:
					break;
			}
		}
		SDL_Delay(33);
	}

	puts("Destroy window");
	SDL_DestroyWindow(window);

	puts("Quit SDL");
	SDL_Quit();

	return EXIT_SUCCESS;
}
