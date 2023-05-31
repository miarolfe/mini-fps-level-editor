#pragma once

#include <map>
#include <string>
#include "SDL.h"
#include "Texture.h"

class Application {
public:
    bool LoadTextureFromFile(Texture& texture, const char* fileName);
    Application(int width, int height);
    void ReassignTextures();
    void AssignNewTextures();
    void ResetLevelMatrix();
    void NewLevel();
    bool SaveLevel(const char* filePath);
    bool LoadLevel(const char* filePath);
private:
    int mapWidth = 32;
    int mapHeight = 32;
    short** levelMatrix;
    std::vector<Texture> textures;
    std::map<std::string, short> textureNameToTextureIdMap;
    std::map<short, Texture> textureIdToTextureMap;
    std::vector<Texture> unassignedTextures;
};