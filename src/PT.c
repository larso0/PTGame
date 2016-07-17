#include <SDL2/SDL.h>
#include "PT.h"

SDL_Window* window;
SDL_GLContext context;
Settings settings;

void Quit()
{
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int Init()
{
    ConstructSettings(&settings);
    LoadSettingsFile(&settings, "settings.ini");
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        Message("Error: Could not initialize SDL.", SDL_GetError());
        return -1;
    }

    if(SDL_GL_LoadLibrary(NULL) < 0)
    {
        Message("Error: Could not load GL library.", SDL_GetError());
        SDL_Quit();
        return -2;
    }

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    window = SDL_CreateWindow
    (
        "PTGame",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        settings.video.width, settings.video.height,
        SDL_WINDOW_OPENGL |
        (settings.video.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP :
                                     SDL_WINDOW_RESIZABLE)
    );
    if(!window)
    {
        Message("Error: Could not create window.", SDL_GetError());
        SDL_Quit();
        return -3;
    }

    context = SDL_GL_CreateContext(window);
    if(!context)
    {
        Message("Error: Could not create GL context.", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -4;
    }

    if(settings.video.vsync &&
       SDL_GL_SetSwapInterval(-1) < 0 &&
       SDL_GL_SetSwapInterval(1) < 0)
    {
        Message("Warning: VSync not supported.", SDL_GetError());
    }

    if(!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        Message("Error: Could not load GL extensions.",
                "Unable to load OpenGL extensions with glad loader.");
        Quit();
        return -5;
    }

    atexit(Quit);
    return 0;
}


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
