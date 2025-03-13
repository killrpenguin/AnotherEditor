#include "App.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstddef>
#include <stdexcept>
#include <utility>
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++20-designator"
namespace
{

const float FONT_SIZE{16.0F};
const int WIDTH{800};
const int HEIGHT{600};
const SDL_Color FG_COLOR{.r = 255, .g = 255, .b = 255, .a = 255};
[[maybe_unused]] const SDL_Color BG_COLOR{.r = 0, .g = 0, .b = 0, .a = 255};
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
    SDL_Log("[LOG]: SDL library initialized.");
    // clang-format off
    if (!SDL_CreateWindowAndRenderer("Hello World!", WIDTH, HEIGHT,
		       SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_INPUT_FOCUS,
                                     &window, &renderer))
    // clang-format on
    {
        throw std::runtime_error("Failed to create window: \n" + std::to_string(*SDL_GetError()));
    }
    SDL_Log("[LOG]: Window and Render created.");

    doc_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);
    if (doc_texture == nullptr)
    {
        throw std::runtime_error("Failed to create document texture: \n" + std::to_string(*SDL_GetError()));
    }

    font_atlas.init(renderer);
    set_line_height();

    SDL_Log("[LOG]: Document texture created.");
    SDL_Log("[LOG]: Application initialized.");
}
auto Application::render_ascii_key(const SDL_Keycode key) noexcept -> void
{
    if (!SDL_SetRenderTarget(renderer, doc_texture))
    {
        SDL_Log("Couldn't set window target: %s", SDL_GetError());
    }

    const SDL_FRect key_pos{font_atlas[key]};

    current_pos.w = key_pos.w;
    current_pos.h = key_pos.h;

    if (!SDL_RenderTexture(renderer, font_atlas.atlas_texture, &key_pos, &current_pos))
    {
        SDL_Log("Couldn't render character to document texture: %c, \n%s", static_cast<char>(key), SDL_GetError());
    }
    if (!SDL_SetRenderTarget(renderer, nullptr))
    {
        SDL_Log("Couldn't set window target: %s", SDL_GetError());
    }
    current_pos.x += key_pos.w;
}
auto Application::draw() -> void
{
    if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1))
    {
        SDL_Log("Failed to set draw color: %s", SDL_GetError());
    }
    if (!SDL_RenderClear(renderer))
    {
        SDL_Log("Failed to clear renderer: %s", SDL_GetError());
    }
    if (!SDL_RenderTexture(renderer, doc_texture, nullptr, nullptr))
    {
        SDL_Log("Failed to render texture: %s", SDL_GetError());
    }
    if (!SDL_RenderPresent(renderer))
    {
        SDL_Log("Failed to present frame: %s", SDL_GetError());
    }
}
constexpr auto Application::set_line_height() noexcept -> void
{
    line_height = font_atlas[SDLK_A].h;
}
auto Application::run() -> void
{
    init();
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
                    render_ascii_key(event.key.key);
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
auto Application::save_image(const std::string &&file_name, SDL_Texture *texture) const noexcept -> void
{
    if (!SDL_SetRenderTarget(renderer, texture))
    {
        SDL_Log("Failed to set render target to requested texture: %s", SDL_GetError());
    }
    SDL_Surface *image{SDL_RenderReadPixels(renderer, nullptr)};
    if (!SDL_SaveBMP(image, file_name.c_str()))
    {
        SDL_Log("Failed to save image: %s", SDL_GetError());
    }
    if (!SDL_SetRenderTarget(renderer, nullptr))
    {
        SDL_Log("Failed to set render target to requested texture: %s", SDL_GetError());
    }
    SDL_DestroySurface(image);
}
auto Application::parse_key(const SDL_Keycode key) noexcept -> void
{
    switch (key)
    {
    case SDLK_ESCAPE:
        running = false;
        save_image("image.bmp", doc_texture);
        break;
    case SDLK_RETURN:
        current_pos.x = 0;
        current_pos.y += line_height;
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
	  SDL_Surface *char_surface = TTF_RenderGlyph_LCD(font, letter, FG_COLOR, BG_COLOR);
	  // 	  SDL_Surface *char_surface = TTF_RenderGlyph_Blended(font, letter, FG_COLOR);
        if (char_surface == nullptr)
        {
            SDL_Log("Failed to create text surface for: %c, \n %s", letter, SDL_GetError());
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
    int letter{32};
    for (auto &surface : surfaces)
    {
        SDL_Rect dst_rectangle{xOffset, 0, surface->w, surface->h};
        SDL_FRect map_rect{};

        SDL_RectToFRect(&dst_rectangle, &map_rect);
        SDL_BlitSurface(surface, nullptr, atlas_surface, &dst_rectangle);

        glyph_data[static_cast<SDL_Keycode>(letter)] = map_rect;
        letter++;
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
