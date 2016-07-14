#ifndef UTIL_H_
#define UTIL_H_

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <stdlib.h>
#include "Shaders.h"
#include "Node.h"
#include "Camera.h"

extern SDL_Window* window;
extern SDL_GLContext context;

int Init();

void Message(const char* title, const char* msg);

#endif
