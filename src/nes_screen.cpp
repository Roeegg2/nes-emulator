#include "../include/nes_screen.h"

namespace roee_nes {

    NES_Screen::NES_Screen(struct Controller* controller1, struct Controller* controller2) {
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
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    controller1->live_status_reg.buttons.up = 1;
                    break;
                case SDLK_DOWN:
                    controller1->live_status_reg.buttons.down = 1;
                    break;
                case SDLK_LEFT:
                    controller1->live_status_reg.buttons.left = 1;
                    break;
                case SDLK_RIGHT:
                    controller1->live_status_reg.buttons.right = 1;
                    break;
                case SDLK_SPACE:
                    controller1->live_status_reg.buttons.select = 1;
                    break;
                case SDLK_RETURN:
                    // std::cout << "got here!" << std::endl;
                    controller1->live_status_reg.buttons.start = 1;
                    break;
                case SDLK_q:
                    controller1->live_status_reg.buttons.b = 1;
                    break;
                case SDLK_e:
                    controller1->live_status_reg.buttons.a = 1;
                    break;
                default:
                    break;
            }
        }
        // else if (event.type == SDL_KEYDOWN) {
        //     switch(event.button.button)
        // }
    }
}