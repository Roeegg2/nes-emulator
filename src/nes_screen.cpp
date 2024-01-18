#ifndef NESSCREEN_H
#define NESSCREEN_H

#include "../include/nes_screen.h"

namespace roee_nes {
    
    NES_Screen::NES_Screen(){
        if (SDL_Init(SDL_INIT_VIDEO) < 0){
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            exit(1);
        }

        screen = SDL_CreateWindow("GAME",
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    1000, 1000, 0);

    }

    NES_Screen::~NES_Screen(){
        SDL_Quit();
    }

}

#endif