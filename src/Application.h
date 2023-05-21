#pragma once

#include "SDL.h"
#include "Texture.h"

class Application {
public:
    static bool LoadTextureFromFile(Texture* texture, const char* fileName);
    Application(int width, int height);
};