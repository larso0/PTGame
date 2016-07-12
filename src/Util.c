#include "Util.h"
#include <SDL2/SDL.h>

void Message(const char* title, const char* msg)
{
    SDL_ShowSimpleMessageBox
    (
        SDL_MESSAGEBOX_INFORMATION |
        SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
        title,
        msg,
        NULL
    );
}
