#include "App.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <utility>
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++20-designator"
namespace
{

const float FONT_SIZE{12.0F};
const int WIDTH{800};
const int HEIGHT{600};
const SDL_Color FIRST{.r = 255, .g = 255, .b = 255, .a = 255};
const SDL_Color SECOND{.r = 0, .g = 0, .b = 0, .a = 255};
const int GLYPH_SPACING{3};
const size_t VISIBLE_ASCII_RANGE{95};
}; // namespace

constexpr auto Application::is_ascii(const SDL_Keycode key) noexcept -> bool
{
    return key >= SDLK_SPACE && key <= SDLK_PLUSMINUS;
}
constexpr auto Application::quit(const SDL_Event event) noexcept -> bool
{
    return event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE;
}
auto Application::init() -> void
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error("Failed to initialize SDL library: \n" + std::to_string(*SDL_GetError()));
    }
    SDL_Log("[LOG]: SDL library initialized successfully.");

    if (!SDL_CreateWindowAndRenderer("Editor", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer))
    {
        throw std::runtime_error("Failed to create window: \n" + std::to_string(*SDL_GetError()));
    }
    SDL_Log("[LOG]: Window and Render created successfully.");

    doc_surface = SDL_CreateSurface(WIDTH, HEIGHT, SDL_PIXELFORMAT_RGBA32);
    if (doc_surface == nullptr)
    {
        throw std::runtime_error("Failed to create document surface: \n" + std::to_string(*SDL_GetError()));
    }
    SDL_Log("[LOG]: Document surface created successfully.");

    doc_texture = SDL_CreateTextureFromSurface(renderer, doc_surface);
    if (doc_surface == nullptr)
    {
        throw std::runtime_error("Failed to create document texture: \n" + std::to_string(*SDL_GetError()));
    }
    SDL_Log("[LOG]: Document texture created successfully.");
    SDL_Log("[LOG]: Application initialized successfully");
}
auto Application::draw_ascii_key(const SDL_Keycode key) const noexcept -> void
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (!SDL_RenderTexture(renderer, font_atlas.atlas_texture, &font_atlas[key], &current_pos))
    {
        SDL_Log("Failed to render texture: %s", SDL_GetError());
        SDL_ClearError();
    }
    SDL_RenderPresent(renderer);
}
auto Application::draw() -> void
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (!SDL_RenderTexture(renderer, doc_texture, nullptr, nullptr))
    {
        SDL_Log("Failed to render texture: %s", SDL_GetError());
        SDL_ClearError();
    }
    SDL_RenderPresent(renderer);
}
auto Application::run() -> void
{
    init();
    font_atlas.init(renderer);
    SDL_Event event{};
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_KEY_DOWN:
                if (Application::is_ascii(event.key.key))
                {
				  draw_ascii_key(event.key.key);
                }
                else
                {
                    parse_key(event.key.key);
                }
                break;
            default:
                break;
            }
        }
        draw();
    }
}
auto Application::parse_key(const SDL_Keycode key) noexcept -> void
{
    switch (key)
    {
    case SDLK_ESCAPE:
        running = false;
        break;
	default:
	  break;
    }
}
/////////////////////////////////////////////////////////////////////////////////
// FontAtlas
/////////////////////////////////////////////////////////////////////////////////
FontAtlas::FontAtlas(std::string &&font_loc) : font_loc{std::move(font_loc)}
{
    if (!TTF_Init())
    {
        throw std::runtime_error("Could not initialize TTF library: \n" + std::to_string(*SDL_GetError()));
    }
    glyph_data.reserve(VISIBLE_ASCII_RANGE);
}
auto FontAtlas::init(SDL_Renderer *renderer) -> void
{
    font = TTF_OpenFont(font_loc.c_str(), FONT_SIZE);
    if (font == nullptr)
    {
        throw std::runtime_error("Failed to open font: \n" + std::to_string(*SDL_GetError()));
    }

    Vec<SDL_Surface *> surfaces{};
    surfaces.reserve(VISIBLE_ASCII_RANGE);

    for (char letter{32}; letter < 127; ++letter)
    {
        SDL_Surface *char_surface = TTF_RenderGlyph_LCD(font, letter, FIRST, SECOND);
        if (char_surface == nullptr)
        {
            std::cerr << "Failed to create text surface for: " << letter << "\n" << *SDL_GetError() << "\n";
            continue;
        }
        surfaces.push_back(char_surface);
    }

    for (auto &surf : surfaces)
    {
        total_width += surf->w + GLYPH_SPACING;
        max_height = std::max(max_height, surf->h);
    }
    texture_atlas_info = SDL_FRect{5, 50, static_cast<float>(total_width), static_cast<float>(max_height)};
    atlas_surface = SDL_CreateSurface(total_width, max_height, SDL_PIXELFORMAT_RGBA32);

    if (atlas_surface == nullptr)
    {
        throw std::runtime_error("Failed to create combined surface: " + std::to_string(*SDL_GetError()));
    }
    int xOffset{0};
    for (auto &surface : surfaces)
    {
        const SDL_Rect dst_rectangle = {xOffset, 0, surface->w, surface->h};
        SDL_BlitSurface(surface, nullptr, atlas_surface, &dst_rectangle);
        xOffset += surface->w + GLYPH_SPACING;
        SDL_DestroySurface(surface);
    }

    atlas_texture = SDL_CreateTextureFromSurface(renderer, atlas_surface);
    if (atlas_texture == nullptr)
    {
        throw std::runtime_error("Failed to create atlas texture: \n" + std::to_string(*SDL_GetError()));
    }
}
// NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
constexpr auto FontAtlas::operator[](const SDL_Keycode key) const noexcept -> const SDL_FRect &
{
    return glyph_data.at(key);
}
constexpr auto FontAtlas::operator[](const SDL_Keycode key) noexcept -> SDL_FRect &
{
    return const_cast<SDL_FRect &>(static_cast<const FontAtlas &>(*this)[key]);
}
// NOLINTEND(cppcoreguidelines-pro-type-const-cast)
// NOLINTEND(cppcoreguidelines-pro-type-vararg,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
