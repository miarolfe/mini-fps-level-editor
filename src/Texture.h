#pragma once

#include "SDL.h"

struct Texture {
    short id;
    std::string name;
    SDL_Texture* sdlTexture;
    SDL_Renderer* sdlRenderer;
    int width, height, channels;
};