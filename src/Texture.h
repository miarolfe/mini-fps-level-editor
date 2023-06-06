#pragma once

#include "SDL.h"

struct Texture {
//    ~Texture() {
//        // Deallocate the texture data
//        // id, name, width, height, channels are automatically deallocated
//        if (sdlTexture != nullptr) {
//            SDL_DestroyTexture(sdlTexture);
//            sdlTexture = nullptr;
//        }
//    }

    short id;
    std::string name;
    SDL_Texture* sdlTexture;
    SDL_Renderer* sdlRenderer;
    int width, height, channels;
};

