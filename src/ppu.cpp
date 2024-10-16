#include <iostream>

#include "../include/mappers/mmc3_4.h"
#include "../include/ppu.h"

namespace roee_nes {
PPU::PPU(NES_Screen *screen)
    : v({0}), t({0}), x(0), w(0), bg_regs({0}), ext_regs({0}), oamdma(0),
      curr_scanline(0), curr_cycle(0), nmi(0), frame_oddness(0),
      frame_counter(0), sprite_rendering_stage(SPRITE_EVAL), render_pixel({0}),
      sprites({0}), primary_oam({0}), secondary_oam({0}), pri_oam_cnt(0),
      sec_oam_cnt(0), curr_sprite_0(false), next_sprite_0(false) {

  this->bus = bus;
  this->screen = screen;
}

void PPU::run_ppu(const uint8_t cycles) {
  for (uint8_t i = 0; i < cycles; i++) {
    if (curr_scanline == PRE_RENDER_SCANLINE)
      prerender_scanline();
    else if ((RENDER_START_SCANLINE <= curr_scanline) &&
             (curr_scanline <= RENDER_END_SCANLINE))
      visible_scanline();
    else if ((VBLANK_START_SCANLINE <= curr_scanline) &&
             (curr_scanline <= VBLANK_END_SCANLINE))
      vblank_scanline();

    if ((bus->mapper->mapper_number == 4) && (!checked))
      check_mmc3_irq_conditions();

    increment_cycle(1);
  }
}

void PPU::check_mmc3_irq_conditions() {
  if (ext_regs.ppuctrl.comp.sprite_size == 0) {
    if (ext_regs.ppuctrl.comp.bg_pt == 0) {
      if (ext_regs.ppuctrl.comp.sprite_pt == 1) {
        if (curr_cycle == 260)
          ((MMC3_4 *)bus->mapper)->clock_irq();
      }
    } else {
      if (ext_regs.ppuctrl.comp.sprite_pt == 0) {
        if (curr_cycle == 324)
          ((MMC3_4 *)bus->mapper)->clock_irq();
      }
    }
  }
  // else {
  //     if ((ext_regs.ppuctrl.comp.bg_pt == 0)
  //         && (ext_regs.ppuctrl.comp.sprite_pt == 1))
  //         // && (/* curr_sprite_overflow */))
  //         ((MMC3_4*)bus->mapper)->clock_irq();
  // }

  checked = true;
  /*
  clock_IRQ:
   if a12 goes from 0 to 1
          if (counter == 0 || reload_flag == true)
              counter = reload_value
          else
              counter--

  */
}

void PPU::shared_visible_prerender_scanline() {
  if ((curr_cycle - 1) % 8 == 0)
    load_shift_regs(); // on cycles 9, 17, 25, etc load the data in the latches
                       // into shift registers

  if ((curr_cycle % 2) ==
      1) { // if we are on an odd frame, that means we need to fetch something
    if (((1 <= curr_cycle) && (curr_cycle <= 256)) ||
        ((321 <= curr_cycle) && (curr_cycle <= 336)))
      fetch_bg_rendering_data(REGULAR_FETCH);
    else if ((337 == curr_cycle) ||
             (curr_cycle == 339)) // between 337 and 340 we only fetch nt
      fetch_bg_rendering_data(ONLY_NT_FETCH); // fetch nt0, nt1
  }

  if (((1 <= curr_cycle) && (curr_cycle <= 256)) ||
      ((321 <= curr_cycle) && (curr_cycle <= 336))) {
    if (ext_regs.ppumask.raw &
        0b00011000) { // if either foreground or background rendering is enabled
      update_shift_regs(); // every cycle we shift the shift registers
      if ((curr_cycle % 8) == 0)
        increment_coarse_x();
    }
  }

  if ((ext_regs.ppumask.raw & 0b00011000) && (curr_cycle == 256))
    increment_y();

  if ((ext_regs.ppumask.raw & 0b00011000) &&
      (curr_cycle == 257)) { // copy all horizontal bits from t onto v
    v.scroll_view.coarse_x = t.scroll_view.coarse_x;
    v.scroll_view.nt = (v.scroll_view.nt & 0b10) | (t.scroll_view.nt & 0b01);
  }

  if ((257 <= curr_cycle) &&
      (curr_cycle <= 320)) // setting oamaddr to 0 during each of these ticks
    ext_regs.oamaddr = 0;
}

void PPU::prerender_scanline() {
  if (curr_cycle == 339) {
    screen->update_screen();
    frame_oddness = 1 - frame_oddness;
    frame_counter++;

    if ((ext_regs.ppumask.raw & 0b00011000) &&
        (frame_oddness ==
         EVEN_FRAME)) { // each ODD scanline we skip a frame. I check for EVEN
                        // scanline because i switch the scanline value before
                        // the 'if'
      increment_cycle(1);
      return;
    }
  }

  if (curr_cycle == 1) {
    ext_regs.ppustatus.raw &=
        0b0001'1111; // clearing vblank, sprite 0 hit, and sprite overflow flag
  }

  if ((ext_regs.ppumask.raw & 0b00011000) && (280 <= curr_cycle) &&
      (curr_cycle <= 304)) {
    v.scroll_view.coarse_y = t.scroll_view.coarse_y;
    v.scroll_view.nt = (v.scroll_view.nt & 0b01) | (t.scroll_view.nt & 0b10);
    v.scroll_view.fine_y = t.scroll_view.fine_y;
  }

  shared_visible_prerender_scanline();
}

void PPU::visible_scanline() {
  if (curr_cycle ==
      0) { // idle cycle. in real NES do nothing, for emulation purposes i use
           // this cycle to clear an internal sprite 0 hit flag
    curr_sprite_0 = next_sprite_0;
    next_sprite_0 = false;
    return;
  }

  if ((1 <= curr_cycle) && (curr_cycle <= 256)) {
    add_render_pixel(); // get bg pixel color; get sprite pixel color; decide
                        // which pixel to add to render line
    screen->draw_pixel_line(&render_pixel, curr_scanline, curr_cycle - 1);

    if (curr_cycle == 64) {
      sprite_rendering_stage = SPRITE_EVAL;
      sec_oam_cnt = 0;
      pri_oam_cnt = 0;
      for (auto it = secondary_oam.begin(); it != secondary_oam.end(); it++)
        *it = 0xff;
    } else if (65 <= curr_cycle) { // sprite evaluation
      if ((ext_regs.ppumask.raw &
           0b0001'1000)) { // if either sprites or background rendering is
                           // enabled
        switch (sprite_rendering_stage) {
        case SPRITE_EVAL:
          sprite_evaluation();
          break;
        case SPRITE_OVERFLOW:
          // sprite_overflow_check();
          break;
        case BROKEN_READ:
          break;
        default:
          std::cerr << "UNKNOWN FG 1-256 PART!\n";
          break;
        }
      }
    }
  } else if (curr_cycle == 257) {
    for (auto it = sprites.begin(); it != sprites.end(); it++)
      *it = {0};
    x_to_sprite_map.clear();
    sec_oam_cnt = 7;
  }

  if ((257 <= curr_cycle) && (curr_cycle <= 320))
    fill_sprites_render_data();

  shared_visible_prerender_scanline();
}

void PPU::vblank_scanline() {
  if ((curr_scanline == VBLANK_START_SCANLINE) && (curr_cycle == 1)) {
    ext_regs.ppustatus.raw |= 0b1000'0000; // set vblank flag
    if (ext_regs.ppuctrl.raw & 0b1000'0000)
      nmi = 1;
  }
}

void PPU::sprite_overflow_check() {
  static uint8_t m = 0;
  uint8_t y = primary_oam[(4 * pri_oam_cnt) + m]; // get y of the new sprite
  if ((0 <= (curr_scanline - y)) && ((curr_scanline - y) <= 7)) {
    ext_regs.ppustatus.raw |= 0b0010'0000;
    sprite_rendering_stage = BROKEN_READ;
    pri_oam_cnt++;
  } else {
    m++;
    pri_oam_cnt++;
    if (m == 4) {
      m = 0;
      pri_oam_cnt++;
    }
  }
  if (pri_oam_cnt == 64) // if all sprites evaluated, goto broken read section
    sprite_rendering_stage = BROKEN_READ;
}

void PPU::sprite_evaluation() {
  static uint8_t byte_0;
  if ((curr_cycle % 2) == 1)                     // if cycle is odd
    byte_0 = primary_oam[(4 * pri_oam_cnt) + 0]; // read from primary oam
  else {
    if (sec_oam_cnt < 8) {
      secondary_oam[(4 * sec_oam_cnt) + 0] = byte_0;
      int32_t diff = curr_scanline - byte_0;

      if (((0 <= diff) && (diff <= 7)) ||
          ((ext_regs.ppuctrl.comp.sprite_size == 1) && (8 <= diff) &&
           (diff <= 15))) {
        secondary_oam[(4 * sec_oam_cnt) + 1] =
            primary_oam[(4 * pri_oam_cnt) + 1]; // byte 1 tile
        secondary_oam[(4 * sec_oam_cnt) + 2] =
            primary_oam[(4 * pri_oam_cnt) + 2]; // byte 2 at
        secondary_oam[(4 * sec_oam_cnt) + 3] =
            primary_oam[(4 * pri_oam_cnt) + 3]; // byte 3 x

        if (pri_oam_cnt == 0) { // if sprite 0 is hit, this will be the index;
          next_sprite_0 = true;
          // std::cout << "scanline: " << curr_scanline << "\n";
        }

        sec_oam_cnt++;
      }
    }
    pri_oam_cnt++;
  }
  if (pri_oam_cnt == 64) {
    sprite_rendering_stage = BROKEN_READ;
  }
  if (sec_oam_cnt >= 8) {
    sprite_rendering_stage = SPRITE_OVERFLOW;
  } // if more than 8 sprites have been found
}

void PPU::fill_sprites_render_data() {
  switch ((curr_cycle) % 8) {
  case Y_BYTE_0: // case this is 1
    sprites[sec_oam_cnt].y = secondary_oam[(4 * sec_oam_cnt) + 0];
    break;
  case TILE_BYTE_1: // case this is 2
    sprites[sec_oam_cnt].tile = secondary_oam[(4 * sec_oam_cnt) + 1];
    break;
  case AT_BYTE_2: // case this is 3
    sprites[sec_oam_cnt].at = secondary_oam[(4 * sec_oam_cnt) + 2];
    break;
  case X_BYTE_3: // case this is 4
    sprites[sec_oam_cnt].x = secondary_oam[(4 * sec_oam_cnt) + 3];
    break;
  case FILL_BUFFER: // case this is 5
    fill_sprite_pixels(sec_oam_cnt);
    break;
  case 0:
    sec_oam_cnt = (sec_oam_cnt - 1);
  default: // if its 6,7
    // the PPU should fetch here X byte again 4 times, but for emulation this is
    // unessecary, so do nothing
    break;
  }
}

void PPU::add_to_x_map(const uint8_t pt_data, const uint8_t i_val) {
  sprites[sec_oam_cnt].palette_indices[i_val] =
      ((4 * (sprites[sec_oam_cnt].at & 0b0000'0011)) +
       pt_data); // setting palette value

  auto it = x_to_sprite_map.find(
      sprites[sec_oam_cnt].x +
      i_val); // trying to find the element if it already exists in the map
  if ((it == x_to_sprite_map.end())) // if it doesnt exist
    x_to_sprite_map.emplace(sprites[sec_oam_cnt].x + i_val,
                            sec_oam_cnt); // add it!
  else if (sprites[sec_oam_cnt].palette_indices[i_val] % 0x4 !=
           0) { // if such sprite already exists, but this sprite is transparent
                // then add this one because this one has higher priority
    x_to_sprite_map.erase(it); // erase the old one
    x_to_sprite_map.emplace(sprites[sec_oam_cnt].x + i_val,
                            sec_oam_cnt); // place the new one
  }
}

void PPU::fill_sprite_pixels(const uint8_t sec_oam_cnt) {
  uint8_t pt_low = fetch_fg_pt_byte(PT_LSB, sprites[sec_oam_cnt]);
  uint8_t pt_high = fetch_fg_pt_byte(PT_MSB, sprites[sec_oam_cnt]);

  for (int i = 0; i < 8; i++) {
    uint8_t pt_data = (((pt_high >> (7 - i)) & 0b0000'0001) << 1) |
                      ((pt_low >> (7 - i)) & 0b0000'0001);

    if (sprites[sec_oam_cnt].at &
        0b0100'0000) // if sprite is flipped horizontally
      add_to_x_map(pt_data, 7 - i);
    else
      add_to_x_map(pt_data, i);
  }
}

uint8_t PPU::get_bg_palette_index() {
  auto get_color_bit = [](uint16_t shift_reg_lsb, uint16_t shift_reg_msb,
                          uint8_t x) -> uint8_t {
    uint8_t data = 0;
    data |= (0b0000'0001 & (shift_reg_lsb >> (15 - x)));
    data |= (0b0000'0010 & (shift_reg_msb >> (14 - x)));

    return data;
  };

  uint8_t pt_data =
      get_color_bit(bg_regs.pt_shift_lsb, bg_regs.pt_shift_msb, x);
  uint8_t at_data =
      get_color_bit(bg_regs.at_shift_lsb, bg_regs.at_shift_msb, x);

  return (4 * at_data) + pt_data;
}

uint8_t PPU::fetch_fg_pt_byte(const uint16_t priority, struct Sprite &sprite) {
  uint16_t addr =
      priority; // bits 3,13 setting msb/lsb (13 is constant 0 for now)
  uint8_t tile = sprite.tile;
  int32_t y_diff = curr_scanline - sprite.y;

  if (ext_regs.ppuctrl.comp.sprite_size) { // if this is a 8x16 sprite
    uint16_t temp = (sprite.tile & 0b0000'0001) << 12;
    addr |= temp;

    if (sprite.at & 0b1000'0000) {
      if ((8 <= y_diff) && (y_diff <= 15))
        tile = (sprite.tile & 0b1111'1110);
      else if ((0 <= y_diff) && (y_diff <= 7))
        tile = (sprite.tile & 0b1111'1110) + 1;
    } else {
      if ((0 <= y_diff) && (y_diff <= 7))
        tile = (sprite.tile & 0b1111'1110);
      else if ((8 <= y_diff) && (y_diff <= 15))
        tile = (sprite.tile & 0b1111'1110) + 1;
    }
  } else // if this is a 8x8 sprite
    addr |= ((ext_regs.ppuctrl.raw & 0b0000'1000) << 9); // bit 12

  if (sprite.at & 0b1000'0000)
    addr |=
        (7 - y_diff) & 0b0000'0000'0000'0111; // bits 0,1,2 masking just in case
  else
    addr |= y_diff & 0b0000'0000'0000'0111; // bits 0,1,2 masking just in case

  addr |= (((uint16_t)tile)
           << 4); // bits 4,5,6,7,8,9,10,11 setting the tile to select from
  return bus->ppu_read(addr);
}

void PPU::fetch_bg_rendering_data(const Fetch_Modes fetch_mode) {
  if (fetch_mode == ONLY_NT_FETCH) { // only fetching nametable byte
    bg_regs.nt_latch = bus->ppu_read(0x2000 | (v.raw & 0x0FFF));
    return;
  }

  switch (curr_cycle %
          8) {  // getting 1, 3, 5, or 7 representing what do we need to fetch
  case FETCH_1: // fetch nt
    bg_regs.nt_latch = bus->ppu_read(0x2000 | (v.raw & 0x0FFF));
    break;
  case FETCH_2: // its fetch at
    bg_regs.at_latch = bus->ppu_read(0x23C0 | (v.scroll_view.nt << 10) |
                                     ((v.scroll_view.coarse_y >> 2) << 3) |
                                     (v.scroll_view.coarse_x >> 2));

    if (v.scroll_view.coarse_y & 0x02)
      bg_regs.at_latch >>= 4;
    if (v.scroll_view.coarse_x & 0x02)
      bg_regs.at_latch >>= 2;

    bg_regs.at_latch &= 0b11; // removing the rest of the bits, which are
                              // unnessecary to get only bit 0, 1
    break;
  case FETCH_3: // fetch pt msb
    bg_regs.pt_latch_lsb = fetch_bg_pt_byte(PT_LSB);
    break;
  case FETCH_4: // fetch pt lsb
    bg_regs.pt_latch_msb = fetch_bg_pt_byte(PT_MSB);
    break;
  }
}

void PPU::check_sprite_0_hit(const uint8_t sprite_index,
                             const uint8_t bg_palette_index) {
  if ((ext_regs.ppustatus.comp.sprite_0_hit != 1) &&
      ((curr_cycle - 1) != 255) // 255 doesnt not hit
      && (curr_sprite_0) && (sprite_index == 0) &&
      ((sprites[sprite_index]
            .palette_indices[curr_cycle - 1 - sprites[sprite_index].x] %
        0x4) != 0) &&
      ((bg_palette_index % 4) != 0)) {

    ext_regs.ppustatus.comp.sprite_0_hit = 1;
  }
}

void PPU::add_render_pixel() {
  uint8_t bg_palette_index = get_bg_palette_index();
  auto it = x_to_sprite_map.find(curr_cycle - 1);

  if ((it != x_to_sprite_map.end()) && (curr_scanline != 0) &&
      ((curr_cycle - 1 - sprites[it->second].x) >= 0)) {
    if (((0 <= (curr_cycle - 1)) && ((curr_cycle - 1) <= 7)) &&
        ((!ext_regs.ppumask.comp.fg_leftmost) &&
         (!ext_regs.ppumask.comp
               .bg_leftmost))) // if we are on the left and clipping is disabled
      goto render;             // dont check for sprite 0 hit

    check_sprite_0_hit(it->second, bg_palette_index);

  render:
    if ((0xf9 <= sprites[it->second].x) && (sprites[it->second].x <= 0xff)) {
      get_chosen_pixel(0, bg_palette_index);
      return;
    }

    if (((sprites[it->second].at & 0b0010'0000) == 0) &&
        (((sprites[it->second]
               .palette_indices[curr_cycle - 1 - sprites[it->second].x] %
           0x4) != 0)))
      get_chosen_pixel(
          0x10, sprites[it->second]
                    .palette_indices[curr_cycle - 1 - sprites[it->second].x]);
    else if ((bg_palette_index % 4) == 0)
      get_chosen_pixel(
          0x10, sprites[it->second]
                    .palette_indices[curr_cycle - 1 - sprites[it->second].x]);
    else
      get_chosen_pixel(0, bg_palette_index);
  } else
    get_chosen_pixel(0, bg_palette_index);
}

void PPU::get_chosen_pixel(const uint8_t base, const uint8_t palette_index) {
  // if (base == 0x10) { // FOR TESTING PURPOSES
  //     render_pixel.r = 0xff;
  //     render_pixel.g = 0xff;
  //     render_pixel.b = 0x0;
  //     return;
  // }
  uint8_t color_index = bus->ppu_read(0x3f00 + base + palette_index);
  render_pixel.r = bus->color_palette[(color_index * 3) + 0];
  render_pixel.g = bus->color_palette[(color_index * 3) + 1];
  render_pixel.b = bus->color_palette[(color_index * 3) + 2];
}

uint8_t PPU::fetch_bg_pt_byte(const uint8_t byte_significance) const {
  uint16_t fetch_addr = v.scroll_view.fine_y; // setting bits 0,1,2
  fetch_addr |= byte_significance;            // setting bit 3
  fetch_addr |= bg_regs.nt_latch << 4; // setting bits 4,5,6,7,8,9,10,11,12
  fetch_addr |= (ext_regs.ppuctrl.raw & 0b0001'0000) << 8; // setting bit 13
  fetch_addr &= 0b0011'1111'1111'1111; // bit 14 should always be set to 0

  return bus->ppu_read(fetch_addr);
}

void PPU::load_shift_regs() {
  // fill pattern table shift regs
  bg_regs.pt_shift_lsb = (bg_regs.pt_shift_lsb & 0xFF00) | bg_regs.pt_latch_lsb;
  bg_regs.pt_shift_msb = (bg_regs.pt_shift_msb & 0xFF00) | bg_regs.pt_latch_msb;

  // deciding which 2 bits i need, depending on which nametable we are currently
  // at
  auto fill_shift_reg = [](uint8_t at_latch, uint16_t at_shift_reg,
                           uint8_t significance) {
    if ((at_latch & significance) == significance)
      return (at_shift_reg & 0xff00) | 0x00ff;
    else
      return (at_shift_reg & 0xff00);
  };

  // filling the shift regs
  bg_regs.at_shift_lsb =
      fill_shift_reg(bg_regs.at_latch, bg_regs.at_shift_lsb, 0b01);
  bg_regs.at_shift_msb =
      fill_shift_reg(bg_regs.at_latch, bg_regs.at_shift_msb, 0b10);
}

void PPU::update_shift_regs() {
  if ((ext_regs.ppumask.raw & 0b00011000)) {
    bg_regs.pt_shift_lsb <<= 1;
    bg_regs.pt_shift_msb <<= 1;
    bg_regs.at_shift_lsb <<= 1;
    bg_regs.at_shift_msb <<= 1;
  }
}

/* function to increment the fine y and coarse y component of v */
void PPU::increment_y() {
  if (v.scroll_view.fine_y < 7) // if fine y < 7 (normal incrementing)
    v.scroll_view.fine_y += 1;
  else {
    v.scroll_view.fine_y = 0;
    uint8_t coarse_y = v.scroll_view.coarse_y;
    if (coarse_y == 29) {
      coarse_y = 0;
      v.scroll_view.nt ^= 0b10; // switching vertical nametable
    } else if (coarse_y == 31)
      coarse_y = 0;
    else
      coarse_y += 1;

    v.scroll_view.coarse_y = coarse_y; // placing coarse y back onto v
  }
}

void PPU::increment_coarse_x() {
  if (v.scroll_view.coarse_x == 31) { // if coarse x = 31 (the limit)
    v.scroll_view.coarse_x = 0;       // coarse x = 0
    v.scroll_view.nt ^= 0b01;         // switching horizontal nametable
  } else
    v.scroll_view.coarse_x += 1; // otherwise, increment v normally
}

/* function to increment the curr_cycle internal counter (and curr_scanline
 * accordingly) */
void PPU::increment_cycle(const uint8_t cycles) {
  curr_cycle += cycles;

  if (curr_cycle > 340) {
    curr_cycle = 0;
    curr_scanline++;
    curr_scanline %= 262; // wrapping around 261 -> 0
    checked = false;
  }
}

void PPU::reset() { // change later!
  v.raw = 0;
  t.raw = 0;
  x = 0;
  w = 0;
  curr_cycle = 0;
  curr_scanline = 0;
  frame_oddness = 0;
  nmi = 0;
  bg_regs = {0};
  ext_regs = {0};
  frame_counter = 0;
  oamdma = 0;
  sprite_rendering_stage = SPRITE_EVAL;
  sec_oam_cnt = 0;
  pri_oam_cnt = 0;
}

void PPU::cpu_write_ppu(const uint16_t addr, const uint8_t data) {
  switch (addr % 8) {
  case OAMADDR:
    ext_regs.oamaddr = data;
    break;
  case OAMDATA:
    if (((RENDER_START_SCANLINE <= curr_scanline) &&
         (curr_scanline <= RENDER_END_SCANLINE)) ||
        (curr_scanline == PRE_RENDER_SCANLINE))
      return; // if we are not in vblank, we do nothing. TODO implement the
              // weird increment of oamaddr here
    primary_oam[ext_regs.oamaddr] = data;
    ext_regs.oamaddr += 1;
    break;
  case PPUCTRL:
    // if ( <= 30000) return; // but not really important
    ext_regs.ppuctrl.raw = data;
    t.scroll_view.nt = data & 0b0000'0011;
    break;
  case PPUMASK:
    ext_regs.ppumask.raw = data;
    break;
  case PPUSCROLL:
    if (w == 0) {
      t.scroll_view.coarse_x = data >> 3; // setting coarse x
      x = data & 0b0000'0111;             // setting fine x
    } else {                              // setting coarse y and fine y
      t.scroll_view.fine_y = data & 0b0000'0111;
      t.scroll_view.coarse_y = data >> 3;
    }

    w = 1 - w; // changing w from 1 to 0 and vise versa
    break;
  case PPUADDR:
    if (w == 0) { // setting upper 6 bits data
      t.raw = (t.raw & 0x00ff) |
              ((data & 0x3F)
               << 8); // bit 14 is set to 0 in this case and the register is 15
                      // bits wide, so bit 15 is not set
    } else {          // setting lower byte data
      t.raw = (t.raw & 0x7f00) | data;
      v.raw =
          t.raw; // TODO: do this every 3 cycles to make more games compatible
    }

    w = 1 - w; // changing w from 1 to 0 and vise versa
    break;
  case PPUDATA: // TODO: implement $2007 reads and writes during rendering
                // (incremnting both x and y)
    bus->ppu_write(v.raw & 0x3fff, data,
                   true); // & 0x3fff is to mirror if the addr is out of bounds
    v.raw += (ext_regs.ppuctrl.raw & 0b00000100) ? 32 : 1;
    break;
  }
}

uint8_t PPU::cpu_read_ppu(const uint16_t addr) {
  uint8_t ret = 0;
  switch (addr) {
  case OAMDATA:
    // TODO take care of reading OAMDATA during rendering
    ret = primary_oam[ext_regs.oamaddr];
    break;
  case PPUSTATUS:
    w = 0;
    ret = (ext_regs.ppustatus.raw & 0b11100000) |
          (ppu_stupid_buffer &
           0b00011111); // returning the last 5 bits of the latch and 3 top bits
                        // of the status register
    ext_regs.ppustatus.raw &= 0b01111111; // clearing vblank flag
    break;
  case PPUDATA:
    ret = ppu_stupid_buffer;
    ppu_stupid_buffer = bus->ppu_read(v.raw & 0x3fff, true);
    if (addr >= 0x3f00)
      ret = bus->ppu_read(v.raw & 0x3fff, true);
    v.raw += (ext_regs.ppuctrl.raw & 0b00000100) ? 32 : 1;
    break;
  }

  return ret;
}
} // namespace roee_nes
