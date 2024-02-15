#include "../include/nes_screen.h"

namespace roee_nes {

    NES_Screen::NES_Screen(Controller* controller1, Controller* controller2) {
        this->controller1 = controller1;
        this->controller2 = controller2;

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
            exit(1);
        }

        window = SDL_CreateWindow("NES Emulator",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, 0);

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
        for (int i = 0; i < 256; i++) {
            SDL_SetRenderDrawColor(renderer, (*data_render_line)[i].r, (*data_render_line)[i].g, (*data_render_line)[i].b, 255);
            SDL_RenderDrawPoint(renderer, i, scanline);
        }
    }

    void NES_Screen::update_screen() const {
        SDL_RenderPresent(renderer);

        SDL_RenderDrawPoint(renderer, 0, 0);
    }

    void NES_Screen::handle_events() {
        SDL_PollEvent(&event);

        if (event.type == SDL_QUIT) {
            exit(0);
        } if (event.type == SDL_KEYDOWN) {
            controller1->frame_controls = 0;
            controller1->frame_controls ^= controller1->frame_controls;

            if (event.key.keysym.sym == SDLK_e) // a
                controller1->frame_controls |= 0b0000'0001;
            if (event.key.keysym.sym == SDLK_q) // b
                controller1->frame_controls |= 0b0000'0010;
            if (event.key.keysym.sym == SDLK_SPACE) // select
                controller1->frame_controls |= 0b0000'0100;
            if (event.key.keysym.sym == SDLK_RETURN) // start
                controller1->frame_controls |= 0b0000'1000;
            if (event.key.keysym.sym == SDLK_w) // up
                controller1->frame_controls |= 0b0001'0000;
            if (event.key.keysym.sym == SDLK_s) // down
                controller1->frame_controls |= 0b0010'0000;
            if (event.key.keysym.sym == SDLK_a) // left
                controller1->frame_controls |= 0b0100'0000;
            if (event.key.keysym.sym == SDLK_d) // right
                controller1->frame_controls |= 0b1000'0000;
        }
        if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_e) // a
                controller1->frame_controls &= ~0b0000'0001;
            if (event.key.keysym.sym == SDLK_q) // b
                controller1->frame_controls &= ~0b0000'0010;
            if (event.key.keysym.sym == SDLK_SPACE) // select
                controller1->frame_controls &= ~0b0000'0100;
            if (event.key.keysym.sym == SDLK_RETURN) // start
                controller1->frame_controls &= ~0b0000'1000;
            if (event.key.keysym.sym == SDLK_w) // up
                controller1->frame_controls &= ~0b0001'0000;
            if (event.key.keysym.sym == SDLK_s) // down
                controller1->frame_controls &= ~0b0010'0000;
            if (event.key.keysym.sym == SDLK_a) // left
                controller1->frame_controls &= ~0b0100'0000;
            if (event.key.keysym.sym == SDLK_d) // right
                controller1->frame_controls &= ~0b1000'0000;
        }
    }
}