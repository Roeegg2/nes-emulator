#ifndef NESSCREEN_H
#define NESSCREEN_H

#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <thread>
#include <array>

#include "controller.h"
namespace roee_nes {
    constexpr uint8_t SCALE = 3;
    constexpr uint16_t SCREEN_WIDTH = 256;
    constexpr uint8_t SCREEN_HEIGHT = 240;

    struct Pixel {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    class NES_Screen {
    public:
        NES_Screen(struct Controller* controller1, struct Controller* controller2);
        ~NES_Screen();

        void draw_pixel_line(std::array<struct Pixel, 256>* data_render_line, int32_t scanline);
        void update_screen() const;
        void handle_events();

    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Event event;
    
    private:
        struct Controller* controller1;
        struct Controller* controller2;
    };
}

#endif