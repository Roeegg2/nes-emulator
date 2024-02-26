#include "../include/nes_screen.h"

namespace roee_nes {

    NES_Screen::NES_Screen(Controller* controller1, Controller* controller2) {
        this->controller1 = controller1;
        this->controller2 = controller2;
        sdl_controller1 = NULL;
        sdl_controller2 = NULL;

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK) < 0) {
            std::cerr << "ERROR: SDL could not initialize; SDL_Error: " << SDL_GetError() << "\n";
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

    void NES_Screen::draw_pixel_line(const struct Pixel* render_pixel, const int32_t scanline, const int32_t x_pos) const {
        SDL_SetRenderDrawColor(renderer, render_pixel->r, render_pixel->g, render_pixel->b, 255);
        SDL_RenderDrawPoint(renderer, x_pos, scanline);

    }

    void NES_Screen::update_screen() const {
        SDL_RenderPresent(renderer);

        SDL_RenderDrawPoint(renderer, 0, 0);
    }

    void NES_Screen::handle_events() {
        SDL_PollEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                exit(0);
            case SDL_JOYDEVICEADDED:
                process_joypad_added();
                break;
            case SDL_JOYDEVICEREMOVED:
                process_joypad_removed();
                break;
            case SDL_JOYBUTTONDOWN:
                process_joypad_pressed_buttons(controller1);
                break;
            case SDL_JOYBUTTONUP:
                process_joypad_released_buttons(controller1);
                break;
            case SDL_JOYAXISMOTION:
                process_joypad_dpad_axis_motion(controller1);
                break;
            case SDL_JOYHATMOTION:
                process_joypad_dpad_hat_motion(controller1);
                break;
            case SDL_KEYDOWN:
                process_keyboard_pressed();
                break;
            case SDL_KEYUP:
                process_keyboard_released();
                break;
        }
    }

    void NES_Screen::process_joypad_pressed_buttons(Controller* controller) {
        if (event.jbutton.button == 1) // a
            controller->buttons.comp.a = 1;
        if (event.jbutton.button == 0) // b
            controller->buttons.comp.b = 1;
        if (event.jbutton.button == 8) // select
            controller->buttons.comp.select = 1;
        if (event.jbutton.button == 9) // start 
            controller->buttons.comp.start = 1;
    }

    void NES_Screen::process_joypad_released_buttons(Controller* controller) {
        if (event.jbutton.button == 1) // a
            controller->buttons.comp.a = 0;
        if (event.jbutton.button == 0) // b
            controller->buttons.comp.b = 0;
        if (event.jbutton.button == 8) // select
            controller->buttons.comp.select = 0;
        if (event.jbutton.button == 9) // start
            controller->buttons.comp.start = 0;

    }

    void NES_Screen::process_joypad_dpad_hat_motion(Controller* controller) {
        if (event.jhat.value & SDL_HAT_LEFT)
            controller->buttons.comp.left = 1; // left
        else if (event.jhat.value & SDL_HAT_RIGHT)
            controller->buttons.comp.right = 1; // right
        if (event.jhat.value & SDL_HAT_UP)
            controller->buttons.comp.up = 1; // up
        else if (event.jhat.value & SDL_HAT_DOWN)
            controller->buttons.comp.down = 1; // down

        if (event.jhat.value == SDL_HAT_CENTERED) { // Reset hat
            controller->buttons.comp.left = 0;
            controller->buttons.comp.right = 0;
            controller->buttons.comp.up = 0;
            controller->buttons.comp.down = 0;
        }
    }

    void NES_Screen::process_joypad_dpad_axis_motion(Controller* controller) {
        if (event.type == SDL_JOYAXISMOTION) {
            // Check the axis number and value
            switch (event.jaxis.axis) {
                case 0: // X-axis
                    if (event.jaxis.value < -16000) {
                        controller->buttons.comp.left = 1;
                        controller->buttons.comp.right = 0;
                    } else if (event.jaxis.value > 16000) {
                        controller->buttons.comp.right = 1;
                        controller->buttons.comp.left = 0;
                    } else {
                        controller->buttons.comp.left = 0;
                        controller->buttons.comp.right = 0;
                    }
                    break;
                case 1: // Y-axis
                    if (event.jaxis.value < -16000) {
                        controller->buttons.comp.up = 1;
                        controller->buttons.comp.down = 0;
                    } else if (event.jaxis.value > 16000) {
                        controller->buttons.comp.down = 1;
                        controller->buttons.comp.up = 0;
                    } else {
                        controller->buttons.comp.up = 0;
                        controller->buttons.comp.down = 0;
                    }
                    break;
                    // Add cases for additional axes if needed
            }
        }
    }
    void NES_Screen::process_keyboard_pressed() {
        if (event.key.keysym.sym == SDLK_e) // a
            controller1->buttons.comp.a = 1;
        if (event.key.keysym.sym == SDLK_q) // b
            controller1->buttons.comp.b = 1;
        if (event.key.keysym.sym == SDLK_SPACE) // select
            controller1->buttons.comp.select = 1;
        if (event.key.keysym.sym == SDLK_RETURN) // start
            controller1->buttons.comp.start = 1;
        if (event.key.keysym.sym == SDLK_w) // up
            controller1->buttons.comp.up = 1;
        if (event.key.keysym.sym == SDLK_s) // down
            controller1->buttons.comp.down = 1;
        if (event.key.keysym.sym == SDLK_a) // left
            controller1->buttons.comp.left = 1;
        if (event.key.keysym.sym == SDLK_d) // right
            controller1->buttons.comp.right = 1;
    }

    void NES_Screen::process_keyboard_released() {
        if (event.key.keysym.sym == SDLK_e) // a
            controller1->buttons.comp.a = 0;
        if (event.key.keysym.sym == SDLK_q) // b
            controller1->buttons.comp.b = 0;
        if (event.key.keysym.sym == SDLK_SPACE) // select
            controller1->buttons.comp.select = 0;
        if (event.key.keysym.sym == SDLK_RETURN) // start
            controller1->buttons.comp.start = 0;
        if (event.key.keysym.sym == SDLK_w) // up
            controller1->buttons.comp.up = 0;
        if (event.key.keysym.sym == SDLK_s) // down
            controller1->buttons.comp.down = 0;
        if (event.key.keysym.sym == SDLK_a) // left
            controller1->buttons.comp.left = 0;
        if (event.key.keysym.sym == SDLK_d) // right
            controller1->buttons.comp.right = 0;
    }

    void NES_Screen::process_joypad_added() {
        if (sdl_controller1 == NULL) {
            sdl_controller1 = SDL_JoystickOpen(event.jdevice.which);
            std::cout << "USER INFO: Controller 1 connected\n";
        } else if (sdl_controller2 == NULL) {
            sdl_controller2 = SDL_JoystickOpen(event.jdevice.which);
            std::cout << "USER INFO: Controller 2 connected\n";
        }
    }

    void NES_Screen::process_joypad_removed() {
        SDL_Joystick* removed = SDL_JoystickFromInstanceID(event.jdevice.which);
        if (removed == sdl_controller1) {
            SDL_JoystickClose(removed);
            std::cout << "USER INFO: Controller 1 disconnected\n";
        } else if (removed == sdl_controller2) {
            SDL_JoystickClose(removed);
            std::cout << "USER INFO: Controller 2 disconnected\n";
        }
    }

}