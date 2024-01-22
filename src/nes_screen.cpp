#include "../include/nes_screen.h"

namespace roee_nes {

    NES_Screen::NES_Screen() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            exit(1);
        }

        window = SDL_CreateWindow("NES Emulator",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            256 * 3, 240 * 3, 0);

        renderer = SDL_CreateRenderer(window, -1, 0);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    NES_Screen::~NES_Screen() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void NES_Screen::draw_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
        if (x >= 256 || y >= 240)
            return;

        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawPoint(renderer, x, y);

        if (x == 255 && y == 239)
            SDL_RenderPresent(renderer);
    }
}