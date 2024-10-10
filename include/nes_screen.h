#ifndef NESSCREEN_H
#define NESSCREEN_H

#include <SDL2/SDL.h>
#include <cstdint>

#include "controller.h"
#include "mapper_n_cart.h"

namespace roee_nes {
constexpr uint8_t SCALE = 4;
constexpr uint16_t SCREEN_WIDTH = 256;
constexpr uint8_t SCREEN_HEIGHT = 240;

struct Pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

class NES_Screen {
public:
  NES_Screen(Mapper *const mapper, Controller *const controller_1,
             Controller *const controller_2);
  ~NES_Screen();

  void draw_pixel_line(const struct Pixel *render_pixel, const int32_t scanline,
                       const int32_t x_pos) const;
  void update_screen() const;
  void handle_events();

private:
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;
  SDL_Joystick *sdl_joystick_1;
  SDL_Joystick *sdl_joystick_2;

  Controller *controller_1;
  Controller *controller_2;
  Mapper *mapper;

private:
  Controller *get_controller_pressed() const;
  void process_joypad_pressed_buttons();
  void process_joypad_released_buttons();
  void process_joypad_dpad_axis_motion();
  void process_joypad_dpad_hat_motion();
  void process_keyboard_released();
  void process_keyboard_pressed();
  void process_joypad_added();
  void process_joypad_removed();
};
} // namespace roee_nes

#endif
