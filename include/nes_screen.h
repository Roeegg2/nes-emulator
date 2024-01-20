#ifndef NESSCREEN_H
#define NESSCREEN_H

#include <SDL2/SDL.h>
#include <iostream>
#include <cstdint>
#include <thread>
#include <array>
namespace roee_nes {
    constexpr uint8_t SCALE = 3;
    constexpr uint16_t SCREEN_WIDTH = 256;
    constexpr uint8_t SCREEN_HEIGHT = 240;

    class NES_Screen {
        public:
        NES_Screen();
        ~NES_Screen();

        void draw_pixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);

        private:
        SDL_Window* window;
        SDL_Renderer* renderer;

    };
}

#endif