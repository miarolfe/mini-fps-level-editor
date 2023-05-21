#pragma once

#include "SDL.h"

struct Texture {
    SDL_Texture* sdlTexture;
    SDL_Renderer* sdlRenderer;
    int width, height, channels;
};