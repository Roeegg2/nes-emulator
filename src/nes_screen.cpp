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

        SDL_RenderSetLogicalSize(renderer, 256, 240);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    NES_Screen::~NES_Screen() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void NES_Screen::draw_pixel_line(std::array<struct Pixel, 256>* data_render_line, int32_t scanline) {
        static std::ofstream pixel_file("logs/PIXEL_COLOR_VALUE.log");

        for (int i = 0; i < 256; i++) { 
            SDL_SetRenderDrawColor(renderer, (*data_render_line)[i].r, (*data_render_line)[i].g, (*data_render_line)[i].b, 255);
            SDL_RenderDrawPoint(renderer, i, (int)scanline);
            pixel_file << std::hex << "scanline: " << scanline << " pixel pos: " << i+1 << " r: " << (int)(*data_render_line)[i].r << " g: " << (int)(*data_render_line)[i].g << " b: " << (int)(*data_render_line)[i].b << std::endl;
        }
    }

    void NES_Screen::update_screen() const {
        SDL_RenderPresent(renderer);
    }
}