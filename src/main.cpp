#include <string.h>
#include <iostream>

// #include <SDL2/SDL.h>

#include "../include/cpu.h"
// #include "../include/ppu.h"
#include "../include/bus.h"
#include "../include/utils.h"

int main(){
    uint8_t temp[0xbfe0];

    Bus bus = Bus(Mapper::create_mapper("testr/gameroms/Donkey Kong (U) (PRG1) [!p](1).nes"));
    CPU cpu = CPU(&bus);
    // PPU ppu = PPU(&bus);

    for (int i = 0; i < 10; i++){
        cpu.log();

        cpu.fetch_decode_inst();
        cpu.execute_inst();
    }

    // if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    //     fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    //     return 1;
    // }

    // // Create a window
    // SDL_Window* window = SDL_CreateWindow("Pixel Painter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    // if (!window) {
    //     fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
    //     return 1;
    // }

    // // Create a renderer
    // SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // if (!renderer) {
    //     fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    //     return 1;
    // }

    // // Set the draw color to white
    // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // // Clear the renderer
    // SDL_RenderClear(renderer);

    // // Set the draw color to red
    // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // // Draw a pixel at coordinates (100, 100)
    // SDL_RenderDrawPoint(renderer, 100, 100);

    // // Present the renderer
    // SDL_RenderPresent(renderer);

    // // Wait for a few seconds to see the window
    // SDL_Delay(3000);

    // // Cleanup
    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);
    // SDL_Quit();

    return 0;
}