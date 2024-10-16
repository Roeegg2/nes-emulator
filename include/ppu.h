#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include <unordered_map>

#include "bus.h"
#include "nes_screen.h"

#define Y_DIFF curr_scanline - sprites[sec_oam_cnt].y

namespace roee_nes {
constexpr uint8_t PT_MSB = 0b0000'1000;
constexpr uint8_t PT_LSB = 0b0000'0000;
constexpr uint8_t ODD_FRAME = 1;
constexpr uint8_t EVEN_FRAME = 0;

enum Scanline_Ranges : int16_t {
  PRE_RENDER_SCANLINE = 261,
  RENDER_START_SCANLINE = 0,
  RENDER_END_SCANLINE = 239,
  POST_RENDER_SCANLINE = 240,
  VBLANK_START_SCANLINE = 241,
  VBLANK_END_SCANLINE = 260
};

enum Fetch_Modes : uint8_t { REGULAR_FETCH, ONLY_NT_FETCH };

enum Sprite_Rendering_Modes { SPRITE_EVAL, SPRITE_OVERFLOW, BROKEN_READ };

class Bus;

enum BG_Fetch_Type {
  FETCH_1 = 1,
  FETCH_2 = 3,
  FETCH_3 = 5,
  FETCH_4 = 7,
};

enum FG_Fetch_Type {
  Y_BYTE_0 = 1,
  TILE_BYTE_1 = 2,
  AT_BYTE_2 = 3,
  X_BYTE_3 = 4,
  FILL_BUFFER = 5,
};

struct Background_Regs {
  uint16_t pt_shift_lsb;
  uint16_t pt_shift_msb;

  uint16_t at_shift_lsb;
  uint16_t at_shift_msb;

  uint8_t nt_latch;
  uint8_t at_latch;
  uint8_t pt_latch_lsb;
  uint8_t pt_latch_msb;
};

struct External_Registers {
  union {
    struct {
      uint8_t base_nt : 2;
      uint8_t vram_inc : 1;
      uint8_t sprite_pt : 1;
      uint8_t bg_pt : 1;
      uint8_t sprite_size : 1;
      uint8_t master_slave : 1;
      uint8_t nmi : 1;
    } comp;
    uint8_t raw;
  } ppuctrl;

  union {
    struct {
      uint8_t grayscale : 1;
      uint8_t bg_leftmost : 1;
      uint8_t fg_leftmost : 1;
      uint8_t bg : 1;
      uint8_t fg : 1;
      uint8_t red : 1;
      uint8_t green : 1;
      uint8_t blue : 1;
    } comp;
    uint8_t raw;
  } ppumask;

  union {
    struct {
      uint8_t open_bus : 5;
      uint8_t sprite_overflow : 1;
      uint8_t sprite_0_hit : 1;
      uint8_t vblank : 1;
    } comp;
    uint8_t raw;
  } ppustatus;

  uint8_t oamaddr;
};

typedef union {
  struct Scroll_View {
    uint16_t coarse_x : 5;
    uint16_t coarse_y : 5;
    uint16_t nt : 2;
    uint16_t fine_y : 3;
    uint16_t unused : 1;
  } scroll_view;

  uint16_t raw;
} loopy_reg;

typedef union {
  struct {
    uint8_t m : 2;
    uint8_t n : 6;
  } counter;
  uint8_t raw;
} oam_counter;

struct Sprite {
  uint8_t y;
  uint8_t tile;
  uint8_t at;
  uint8_t x;
  uint8_t palette_indices[8];
};

class PPU {
public:
  PPU(NES_Screen *screen);
  void run_ppu(const uint8_t cycles);
  void reset();

  void cpu_write_ppu(const uint16_t addr, const uint8_t data);
  uint8_t cpu_read_ppu(const uint16_t addr);

private:
  void shared_visible_prerender_scanline();
  void prerender_scanline();
  void visible_scanline();
  void vblank_scanline();
  void sprite_overflow_check();
  void sprite_evaluation();
  void fill_sprites_render_data();
  void add_to_x_map(const uint8_t pt_data, const uint8_t i_val);
  void fill_sprite_pixels(const uint8_t sec_oam_cnt);
  uint8_t get_bg_palette_index();
  uint8_t fetch_fg_pt_byte(const uint16_t priority, struct Sprite &sprite);
  void fetch_bg_rendering_data(const Fetch_Modes fetch_mode);
  void check_sprite_0_hit(const uint8_t sprite_index,
                          const uint8_t bg_palette_index);
  void add_render_pixel();
  void get_chosen_pixel(const uint8_t base, const uint8_t palette_index);
  uint8_t fetch_bg_pt_byte(const uint8_t byte_significance) const;
  void load_shift_regs();
  void update_shift_regs();
  void increment_y();
  void increment_coarse_x();
  void increment_cycle(const uint8_t cycles);
  void check_mmc3_irq_conditions();
#ifdef DEBUG
  void log() const;
#endif
public:
  loopy_reg v;
  loopy_reg t;
  uint8_t x;
  uint8_t w;

  struct Background_Regs bg_regs;
  struct External_Registers ext_regs;
  uint8_t oamdma;

  int32_t curr_scanline;
  int32_t curr_cycle;
  uint8_t nmi;
  uint8_t frame_oddness;
  uint64_t frame_counter;

  uint8_t sprite_rendering_stage;

  // std::array<struct Pixel, 256> data_render_buffer;
  struct Pixel render_pixel;
  std::array<struct Sprite, 8> sprites;
  std::unordered_map<uint8_t, uint8_t> x_to_sprite_map;
  std::array<uint8_t, 256> primary_oam;
  std::array<uint8_t, 32> secondary_oam;

  uint8_t pri_oam_cnt;
  uint8_t sec_oam_cnt;
  bool curr_sprite_0;
  bool next_sprite_0;
  bool checked;
  uint8_t ppu_stupid_buffer;

public:
  class Bus *bus;
  NES_Screen *screen;
};
} // namespace roee_nes

#endif
