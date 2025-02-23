#include "App.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <iomanip>
#include <iostream>
#include <stdexcept>
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
namespace
{

const float FONT_SIZE{24.0F};
const int WIDTH{800};
const int HEIGTH{600};
const SDL_Color FIRST{.r = 255, .g = 255, .b = 255, .a = 255};
const SDL_Color SECOND{.r = 0, .g = 0, .b = 0, .a = 255};
const int GLYPH_SPACING{3};
}; // namespace

constexpr auto Application::is_digit(const SDL_Event &event) noexcept -> bool
{
    return event.key.key >= SDLK_0 && event.key.key >= SDLK_9;
}
constexpr auto Application::is_letter(const SDL_Event &event) noexcept -> bool
{
    return event.key.key >= SDLK_A && event.key.key >= SDLK_Z;
}
constexpr auto Application::is_ascii(const SDL_Event &event) noexcept -> bool
{
    return event.key.key >= SDLK_SPACE && event.key.key >= SDLK_PLUSMINUS;
}
constexpr auto Application::quit(const SDL_Event &event) noexcept -> bool
{
    return event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE;
}
auto Application::clear_screen() const noexcept -> void
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}
auto Application::init() -> void
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    surfaces.reserve(text.size());

    window = SDL_CreateWindow("Editor", WIDTH, HEIGTH, SDL_WINDOW_RESIZABLE);
    if (window == nullptr)
    {
        throw std::runtime_error("Failed to create window: \n" + std::to_string(*SDL_GetError()));
    }
    SDL_Log("[LOG]: Window loaded.");

    renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr)
    {
        throw std::runtime_error("Failed to create SDL renderer: \n" + std::to_string(*SDL_GetError()));
    }
    SDL_Log("[LOG]: Renderer loaded.");

    font = TTF_OpenFont("resources/Inter-VariableFont.ttf", FONT_SIZE);
    if (font == nullptr)
    {
        throw std::runtime_error("Failed to create open font: \n" + std::to_string(*SDL_GetError()));
    }
    SDL_Log("[LOG]: Font loaded.");

    SDL_Log("[LOG]: Application initialized successfully");
}
auto Application::render() -> void
{
    for (const char letter : text)
    {
        SDL_Surface *text_surface = TTF_RenderGlyph_LCD(font, letter, FIRST, SECOND);

        if (text_surface == nullptr)
        {
            std::cerr << "Failed to create text surface: " << *SDL_GetError() << "\n";
            continue;
        }
        surfaces.push_back(text_surface);
    }
    int total_width{0};
    int max_height{0};

    for (auto &surface : surfaces)
    {
        total_width += surface->w + GLYPH_SPACING;
        max_height = std::max(max_height, surface->h);
    }
    rectangle = {.x = 5, .y = 50, .w = static_cast<float>(total_width), .h = static_cast<float>(max_height)};

    SDL_Surface *combined_surface = SDL_CreateSurface(total_width, max_height, SDL_PIXELFORMAT_RGBA32);
    if (combined_surface == nullptr)
    {
        throw std::runtime_error("Failed to create combined surface: " + std::to_string(*SDL_GetError()));
    }

    int x_offset{0};
    for (auto &surface : surfaces)
    {
        const SDL_Rect dest_rectangle = {x_offset, 0, surface->w, surface->h};
        SDL_BlitSurface(surface, nullptr, combined_surface, &dest_rectangle);
        x_offset += surface->w + GLYPH_SPACING;
        SDL_DestroySurface(surface);
    }

    finished_texture = SDL_CreateTextureFromSurface(renderer, combined_surface);
    SDL_DestroySurface(combined_surface);
}
auto Application::draw() const noexcept -> void
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderTexture(renderer, finished_texture, NULL, &rectangle);
    SDL_RenderPresent(renderer);
}
auto Application::run() -> void
{
    init();
    render();
    SDL_Event event{};
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (Application::quit(event))
            {
                running = false;
            }
        }
        draw();
    }
}
// NOLINTEND(cppcoreguidelines-pro-type-vararg,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
