#include "Application.h"
#include "Texture.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cstdio>
#include <SDL.h>


// Dear ImGui uses SDL_Texture* as ImTextureID
bool Application::LoadTextureFromFile(Texture* texture, const char* fileName) {
    unsigned char* data = stbi_load(fileName, &texture->width, &texture->height, &texture->channels, 0);

    if (data == nullptr) {
        fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
        return false;
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)data, texture->width, texture->height, texture->channels * 8, texture->channels * texture->width,
                                                    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    if (surface == nullptr) {
        fprintf(stderr, "Failed to create SDL surface: %s\n", SDL_GetError());
        return false;
    }

    texture->sdlTexture = SDL_CreateTextureFromSurface(texture->sdlRenderer, surface);

    if (texture->sdlTexture == nullptr) {
        fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());
    }

    SDL_FreeSurface(surface);
    stbi_image_free(data);

    return true;
}

Application::Application(int width, int height) {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    SDL_Log("Current SDL_Renderer: %s", info.name);

    // Setup Dear ImGui context
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    // Our state
    int tileSize = 16;
    int mapWidth = 32;
    int mapHeight = 32;
    ImVec4 clear_color = ImVec4(0, 0, 0, 1.00f);

    Texture testTexture{};
    testTexture.sdlRenderer = renderer;
    Application::LoadTextureFromFile(&testTexture, "test.png");

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Create a main window for the tile map
        ImGui::Begin("Map editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);

        // Display the tile map
        for (int y = 0; y < mapHeight; ++y)
        {
            for (int x = 0; x < mapWidth; ++x)
            {
                // Draw the tile at (x, y)
                ImGui::Image(testTexture.sdlTexture, ImVec2(tileSize, tileSize));

                // If the tile was clicked
                if (ImGui::IsItemClicked())
                {
                    fprintf(stderr, "(%d, %d) clicked\n", x, y);
                }

                ImGui::SameLine();
            }

            ImGui::NewLine();
        }

        ImGui::PopStyleVar(3);

        ImGui::End();

        // Create a sidebar for tile and tool selection
        ImGui::Begin("Tools");

        ImGui::SliderInt("Tile size", &tileSize, 1, 32);
        ImGui::SliderInt("Map width", &mapWidth, 3, 128);
        ImGui::SliderInt("Map height", &mapHeight, 3, 128);

        ImGui::End();

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}