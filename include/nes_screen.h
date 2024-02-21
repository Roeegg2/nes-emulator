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
        uint8_t pt_data : 2;
        uint8_t : 6;
    };

    struct Entity_Pixel : public Pixel {
        uint8_t im_sprite_0 : 1;
        uint8_t priority : 1;
        uint8_t : 6;

        struct Entity_Pixel operator=(const int val) {
            struct Entity_Pixel ret;
            ret.r = val;
            ret.g = val;
            ret.b = val;
            ret.pt_data = val;
            return ret;
        }
    };

    class NES_Screen {
        public:
        NES_Screen(Controller* controller1, Controller* controller2);
        ~NES_Screen();

        void draw_pixel_line(std::array<struct Pixel, 256>* data_render_line, int32_t scanline);
        void update_screen() const;
        void handle_events();

        private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Event event;
        SDL_Joystick* sdl_controller1;
        SDL_Joystick* sdl_controller2;

        Controller* controller1;
        Controller* controller2;

        private:
        void process_joypad_pressed_buttons(Controller* controller);
        void process_joypad_released_buttons(Controller* controller);
        void process_joypad_dpad_axis_motion(Controller* controller);
        void process_joypad_dpad_hat_motion(Controller* controller);
        void process_keyboard_released();
        void process_keyboard_pressed();
        void process_joypad_added();
        void process_joypad_removed();
    };
}

#endif