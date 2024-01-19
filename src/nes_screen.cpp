#include "../include/nes_screen.h"

namespace roee_nes
{

    NES_Screen::NES_Screen()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            exit(1);
        }

        window = SDL_CreateWindow("NES Emulator",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  256 * 3, 240 * 3, 0);

        renderer = SDL_CreateRenderer(window, -1, 0);
    }

    NES_Screen::~NES_Screen()
    {
        SDL_Quit();
    }

    void NES_Screen::draw_pixel(uint32_t color_value, uint8_t x, uint8_t y)
    {
        uint8_t r = (color_value & 0xFF000000) >> 24;
        uint8_t g = (color_value & 0x00FF0000) >> 16;
        uint8_t b = (color_value & 0x0000FF00) >> 8;

        if (x >= 256 || y >= 240)
            return;

        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawPoint(renderer, x, y);
        SDL_RenderPresent(renderer);

        // for (uint8_t i = 0; i < SCALE; i++)
        // {
        //     for (uint8_t j = 0; j < SCALE; j++)
        //     {
        //         SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        //         SDL_RenderDrawPoint(renderer, x + i, y + j);
        //     }
        // }
    }

}