// void print_debugger() {
    // printf("------------- DEBUGGER STARTED --------------\n\n");

    // printf("COMMANDS:\n");
    // printf("c/run - continue\n");
    // printf("s - step\n");
    // printf("b <addr> - add breakpoint at <addr>\n");
    // printf("pc - print cpu registers (NOTE: not implemented yet)\n");
    // printf("pp - print ppu registers (NOTE: not implemented yet)\n");
    // printf("pm <addr> - print memory at <addr> (NOTE: not implemented yet)\n");
    // printf("q - quit\n");
// }

#include "debugger_main.h"

extern uint16_t emulator_tick();

static struct Breakpoint* get_breakpoint(struct Breakpoint* head, uint16_t addr) {
    while (head != nullptr)
        if (head->addr == addr)
            return head;
    
    return nullptr;
}

static void add_breakpoint(struct Breakpoint** head, uint16_t addr) {
    struct Breakpoint* new_node = new struct Breakpoint();

    if (*head == nullptr)
        *head = new_node;
    else {
        struct Breakpoint* foo = *head;
        while (foo->next != nullptr)
            foo = foo->next;
        
        foo->next = new_node;
    }

    new_node->addr = addr;
    new_node->next = nullptr;
}

static void draw_pixel(uint32_t color, uint8_t nt_number, uint8_t scroll_y, uint8_t scroll_x, uint8_t fine_y, uint8_t fine_x) {
    static int counter = 0;
    static SDL_Window* window = SDL_CreateWindow("NES Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        256 * 2, 256 * 2, 0);
    static SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    auto get_nt_base_x = [](uint8_t nt_number) -> uint8_t { return (nt_number & 0b00000001) * 256; };
    auto get_nt_base_y = [](uint8_t nt_number) -> uint8_t { return ((nt_number & 0b00000010) >> 1) * 240; };

    SDL_RenderDrawPoint(renderer,
        get_nt_base_x(nt_number) + scroll_x + fine_x,
        get_nt_base_y(nt_number) + scroll_y + fine_y);

    SDL_SetRenderDrawColor(renderer, 255, color & 0x000f, color & 0x00f0, color & 0x0f00);

    if (counter == 1000) {
        SDL_RenderPresent(renderer); // TODO: do this every frame instead of every cycles
        counter = 0;
    }
    counter++;
    std::cout << std::hex << "color value is " << (int)color << std::endl;
}

static uint32_t get_draw_data(uint8_t pt_data) {
    static std::ifstream pal_file("testr/gameroms/palette.pal", std::ios::binary);
    constexpr uint8_t bg_frame_palette_index = 0; // NOTE: currently a const, just to get the shape of the sprites

    char r;
    char g;
    char b;
    pal_file.seekg((bg_frame_palette_index * 4) + 0); // seek to the right entry
    pal_file.read(&r, 1); // get the color
    pal_file.read(&g, 1); // get the color
    pal_file.read(&b, 1); // get the color

    return ((0x000f & r) << 16) || ((0x000f & g) << 8) || ((0x000f & b));
}

static void display_mem(std::array<std::array<uint8_t, 0x400>, 4>* vram) {
    using vram_it = std::array<uint8_t, 0x400>::iterator;

    vram_it it;
    for (int i = 0; i < 4; i++) { // for every nametable (there are 4 nametables)
        for (it = (*vram)[i].begin(); it != (*vram)[i].end(); it++) { // for every entry in the nametable (for every tile)
            uint8_t scroll_x, scroll_y;
            if (std::distance((*vram)[i].begin(), it) % 31 == 0) { // getting the position in the nametable, when visulized as a table
                scroll_y++;
                scroll_x = 0;
            } else
                scroll_x++;

            for (int fine_y = 0; fine_y < 7; fine_y++) { //  for every row of pixel
                for (int fine_x = 0; fine_x < 7; fine_x++) { // for every pixel in that row
                    uint32_t color = get_draw_data(*it);
                    draw_pixel(color, i, scroll_y, scroll_x, fine_y, fine_x);
                }
            }
        }
    }
}

void debugger_run(std::array<std::array<uint8_t, 0x400>, 4>* vram) {
    static uint16_t curr_addr;
    static struct Breakpoint* head = nullptr;

    static std::string last_input;
    std::string input;

    std::cout << "\n>";
    std::cin >> input;

    // if (input.compare("\n"))
    //     input = last_input;
    if (input.compare("step") == 0 || input.compare("s") == 0)
        curr_addr = emulator_tick();
    else if (input.compare("continue") == 0 || input.compare("c") == 0) {
        while (get_breakpoint(head, curr_addr) == nullptr) { // not the most efficient way, but whatever
            curr_addr = emulator_tick();
            display_mem(vram);
        }
    } else if (input.compare("break") == 0 || input.compare("b") == 0) {
        uint16_t addr;
        std::cin >> addr;
        add_breakpoint(&head, addr);
    } else if (input.compare("quit") == 0 || input.compare("q") == 0) {
        std::cout << "Quitting..." << std::endl;
        exit(1);
    } else
        std::cerr << "no such command!" << std::endl;

    display_mem(vram);
}