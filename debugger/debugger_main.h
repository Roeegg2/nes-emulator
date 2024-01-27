#include <iostream>
#include <string>
#include <array>
#include <cstdint>
#include <fstream>

#include <SDL2/SDL.h>

enum class Commands {
    CONTINUE,
    BREAK,
    STEP,
    NO_COMMAND
};

struct Breakpoint {
    uint16_t addr;

    struct Breakpoint* next;
};

static struct Breakpoint* get_breakpoint(struct Breakpoint* head, uint16_t addr);
static void add_breakpoint(struct Breakpoint** head, uint16_t addr);
static void draw_pixel(uint8_t color, uint8_t nt_number, uint8_t scroll_y, uint8_t scroll_x, uint8_t fine_y, uint8_t fine_x);
static uint32_t get_draw_data(uint8_t pt_data);
static void display_mem();
void debugger_run();