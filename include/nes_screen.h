#include <SDL2/SDL.h>
#include <iostream>

namespace roee_nes {
    
    class NES_Screen {
    public:
        NES_Screen();
        ~NES_Screen();

        void draw_pixel();

    private:
        SDL_Window* screen;
    
    };
}