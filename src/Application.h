#pragma once

#include <map>
#include <string>
#include "SDL.h"
#include "Texture.h"

class Application {
public:
    bool LoadTextureFromFile(Texture* texture, const char* fileName);
    Application(int width, int height);
    void ResetLevelMatrix();
    void NewLevel();
    bool SaveLevel(const char* filePath);
    bool LoadLevel(const char* filePath);
private:
    int mapWidth = 32;
    int mapHeight = 32;
    short** levelMatrix;
    std::map<std::string, short> textureNameToTextureIdMap;
};