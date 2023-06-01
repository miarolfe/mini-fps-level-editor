#include "Application.h"
#include "Texture.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer.h"
#include "portable-file-dialogs.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cstdio>
#include <fstream>
#include <SDL.h>

// Dear ImGui uses SDL_Texture* as ImTextureID
bool Application::LoadTextureFromFile(Texture& texture, const char* fileName) {
    texture.id = -1;

    unsigned char* data = stbi_load(fileName, &texture.width, &texture.height, &texture.channels, 0);

    if (data == nullptr) {
        fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
        return false;
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)data, texture.width, texture.height, texture.channels * 8, texture.channels * texture.width,
                                                    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    if (surface == nullptr) {
        fprintf(stderr, "Failed to create SDL surface: %s\n", SDL_GetError());
        return false;
    }

    texture.sdlTexture = SDL_CreateTextureFromSurface(texture.sdlRenderer, surface);

    if (texture.sdlTexture == nullptr) {
        fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());
    }

    SDL_FreeSurface(surface);
    stbi_image_free(data);

    std::string textureName(fileName);

    size_t periodPos = textureName.find('.');
    textureName.erase(periodPos);// Remove characters after the period

    size_t lastSlashPos = textureName.find_last_of('/');
    textureName.erase(0, lastSlashPos + 1);// Remove everything before the last slash

    fprintf(stdout, "Texture name: %s\n", textureName.c_str());

    texture.name = textureName;

    if (textureNameToTextureIdMap.count(textureName) == 1) {
        texture.id = textureNameToTextureIdMap[textureName];
    }

    return true;
}

void Application::ReassignTextures() {
    for (const auto& texture : textures) {
        if (textureNameToTextureIdMap.count(texture.name) == 1) {
            textureIdToTextureMap[textureNameToTextureIdMap[texture.name]] = texture;
        } else {
            unassignedTextures.push_back(texture);
        }
    }
}

void Application::AssignNewTextures() {
    for (const auto& texture : textures) {
        if (texture.id != -1) {
            textureIdToTextureMap[texture.id] = texture;
        }
    }
}

void Application::ResetLevelMatrix() {
    levelMatrix = new short*[mapHeight];

    for (int i = 0; i < mapHeight; i++) {
        levelMatrix[i] = new short[mapWidth];
    }
}

void Application::NewLevel() {
    ResetLevelMatrix();
}

bool Application::SaveLevel(const char* filePath) {
    assert(mapWidth >= 3);
    assert(mapHeight >= 3);

    std::ofstream outfile(filePath);
    if (!outfile) {
        fprintf(stderr, "Error saving level: %s\n", filePath);
        return false;
    }

    outfile << mapWidth << " " << mapHeight << std::endl;
    for (size_t y = 0; y < mapHeight; y++) {
        for (size_t x = 0; x < mapWidth; x++) {
            outfile << levelMatrix[y][x] << (x < mapHeight - 1 ? " " : "");
        }
        outfile << std::endl;
    }

    std::map<short, std::string> reversedMap;

    for (const auto& entry : textureNameToTextureIdMap) {
        reversedMap[entry.second] = entry.first;
    }

    for (const auto& entry: reversedMap) {
        outfile << entry.first << " " << entry.second << std::endl;
    }

    return true;
}

bool Application::LoadLevel(const char* filePath) {
    std::ifstream infile(filePath);
    if (!infile) {
        fprintf(stderr, "Error loading level: %s\n", filePath);
        return false;
    }

    infile >> mapWidth;
    infile >> mapHeight;

    ResetLevelMatrix();
    textureNameToTextureIdMap.clear();
    textureIdToTextureMap.clear();
    unassignedTextures.clear();

    for (int i = 0; i < mapHeight; i++) {
        for (int j = 0; j < mapWidth; j++) {
            infile >> levelMatrix[i][j];
        }
    }

    while (infile.peek() != EOF) {
        short id;
        infile >> id;
        std::string textureName;
        infile >> textureName;
        textureNameToTextureIdMap[textureName] = id;
    }

    ReassignTextures();

    infile.close();

    return true;
}

Application::Application(int width, int height) {
    NewLevel();

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
    }

    SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("mini-fps-level-editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, windowFlags);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    if (renderer == nullptr) {
        SDL_Log("Error creating SDL_Renderer!");
    }

    // Setup Dear ImGui context
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    int editorTileSize = 16;
    int paletteTileSize = 64;
    int currentTile = 0;
    ImVec4 backgroundColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

    pfd::open_file textureFileDialog = pfd::open_file("Select textures", "", {"Image Files", "*.png"}, pfd::opt::multiselect);

    Texture fallbackTexture;
    fallbackTexture.sdlRenderer = renderer;
    Application::LoadTextureFromFile(fallbackTexture, "../Resources/sprites/fallback.png");

    textures.clear();
    textures.reserve(textureFileDialog.result().size());

    for (int i = 0; i < textureFileDialog.result().size(); i++) {
        Texture newTexture;
        newTexture.sdlRenderer = renderer;
        Application::LoadTextureFromFile(newTexture, textureFileDialog.result()[i].c_str());
        textures.push_back(newTexture);
        unassignedTextures.push_back(newTexture);
    }

    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                done = true;
            }

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
                done = true;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        short currentTileShort = static_cast<short>(currentTile);
        float editorTileSizeFloat = static_cast<float>(editorTileSize);
        float paletteTileSizeFloat = static_cast<float>(paletteTileSize);

        ImGui::Begin("Map editor", nullptr);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 1));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);

        // Display the tile map
        for (int y = 0; y < mapHeight; ++y) {
            for (int x = 0; x < mapWidth; ++x) {
                short cellId = levelMatrix[y][x];

                if (cellId != 0 && textureIdToTextureMap.count(cellId) == 1) {
                    ImGui::Image(textureIdToTextureMap[cellId].sdlTexture, ImVec2(editorTileSizeFloat, editorTileSizeFloat));
                } else if (cellId != 0) {
                    ImGui::Image(fallbackTexture.sdlTexture, ImVec2(editorTileSizeFloat, editorTileSizeFloat));
                } else {
                    ImGui::Image(nullptr, ImVec2(editorTileSizeFloat, editorTileSizeFloat));
                }

                // If the tile was clicked
                if (ImGui::IsItemClicked()) {
                    fprintf(stderr, "(%d, %d) clicked\n", x, y);
                    levelMatrix[y][x] = currentTileShort;
                }

                ImGui::SameLine();
            }

            ImGui::NewLine();
        }

        ImGui::PopStyleVar(3);

        ImGui::End();

        ImGui::Begin("Settings", nullptr);

//        int newMapWidth = mapWidth;
//        int newMapHeight = mapHeight;

        ImGui::SliderInt("Current tile", &currentTile, 0, 16);
        ImGui::SliderInt("Editor tile size", &editorTileSize, 8, 64);
        ImGui::SliderInt("Palette tile size", &paletteTileSize, 32, 128);
//        ImGui::SliderInt("Map width", &newMapWidth, 3, 128);
//        ImGui::SliderInt("Map height", &newMapHeight, 3, 128);

        ImGui::End();

        ImGui::Begin("Palette", nullptr);
        ImGui::Text("Assigned textures");

        for (const auto& entry : textureIdToTextureMap) {
            ImGui::Text("id: %d", entry.first);
            ImGui::SameLine();
            ImGui::Text("name: %s", entry.second.name.c_str());

            ImGui::Image(entry.second.sdlTexture, ImVec2(paletteTileSizeFloat, paletteTileSizeFloat));
            if (ImGui::IsItemClicked()) {
                fprintf(stdout, "Image %d clicked\n", entry.first);
                currentTile = entry.first;
            }
            ImGui::NewLine();
        }
        ImGui::NewLine();

        short newlyAssignedTextureId = -1;

        if (!unassignedTextures.empty()) {
            ImGui::Text("Unassigned textures");
        }

        for (int i = 0; i < unassignedTextures.size(); i++) {
            ImGui::Text("name: %s", unassignedTextures[i].name.c_str());
            ImGui::Image(unassignedTextures[i].sdlTexture, ImVec2(paletteTileSizeFloat, paletteTileSizeFloat));
            if (ImGui::IsItemClicked()) {
                short id = -1;
                short potentialId = 1;

                while (id == -1) {
                    if (textureIdToTextureMap.count(potentialId) == 0) {
                        id = potentialId;
                        textureIdToTextureMap[id] = unassignedTextures[i];
                        textureNameToTextureIdMap[unassignedTextures[i].name] = id;
                        unassignedTextures[i].id = id;
                        newlyAssignedTextureId = id;
                    } else {
                        potentialId++;
                    }
                }
            }

            ImGui::NewLine();
        }

        if (newlyAssignedTextureId != -1) {
            int index = -1;

            // Find index in unassigned texture pool
            for (int j = 0; j < unassignedTextures.size(); j++) {
                if (newlyAssignedTextureId == unassignedTextures[j].id) {
                    index = j;
                    fprintf(stdout, "Index is %d\n", index);
                    break;
                }
            }

            // If the index exists, remove the item at index
            if (index != -1) {
                for (int h = index; h < unassignedTextures.size(); h++) {
                    unassignedTextures[h] = unassignedTextures[h+1];
                }
            }

            if (!unassignedTextures.empty()) {
                unassignedTextures.pop_back();
            }

            currentTile = newlyAssignedTextureId;

            AssignNewTextures();
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Level")) {
                if (ImGui::MenuItem("New", "", nullptr)) {
                    NewLevel();
                }

                if (ImGui::MenuItem("Save", "", nullptr)) {
                    pfd::save_file newLevelFileDialog = pfd::save_file("Save level");
                    SaveLevel(newLevelFileDialog.result().c_str());
                }

                if (ImGui::MenuItem("Load", "", nullptr)) {
                    pfd::open_file newLevelFileDialog = pfd::open_file("Load level", "", {"Level Files", "*.lvl"});
                    LoadLevel(newLevelFileDialog.result()[0].c_str());
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Textures")) {
                if (ImGui::MenuItem("Load texture folder", "", nullptr)) {
                    pfd::select_folder selectTextureFolderDialog = pfd::select_folder("Load texture folder");
                    fprintf(stderr, "Loading texture folder not yet implemented\n");
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGui::End();

        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(backgroundColor.x * 255), (Uint8)(backgroundColor.y * 255), (Uint8)(backgroundColor.z * 255), (Uint8)(backgroundColor.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);

//        if (newMapWidth != mapWidth || newMapHeight != mapHeight) {
//            mapWidth = newMapWidth;
//            mapHeight = newMapHeight;
//            NewLevel();
//        }
    }

    // Cleanup
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}