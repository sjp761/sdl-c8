#include <SDL2/SDL.h>
class SDL_MainComponents
{
    public:
        static SDL_Window* window;
        static SDL_Renderer* renderer;
        static void renderUpdate();
        static void init();

};