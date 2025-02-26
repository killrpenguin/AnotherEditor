#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <unordered_map>
#include <vector>

template <typename T> using Vec = std::vector<T>;

class FontAtlas
{
    template <typename K, typename V> using Map = std::unordered_map<K, V>;

  protected:
    TTF_Font *font{};
    std::string font_loc{};

    int total_width{0};
    int max_height{0};

  public:
    friend class Application;

    SDL_Surface *atlas_surface{};
    SDL_Texture *atlas_texture{};

    SDL_FRect texture_atlas_info{};

    Map<SDL_Keycode, SDL_FRect> glyph_data{};
    explicit FontAtlas(std::string &&font_loc);
    ~FontAtlas()
    {
        SDL_DestroySurface(atlas_surface);
        SDL_DestroyTexture(atlas_texture);
        TTF_CloseFont(font);
        TTF_Quit();
    }
    constexpr auto operator[](const SDL_Keycode key) const noexcept -> const SDL_FRect &;
    constexpr auto operator[](const SDL_Keycode key) noexcept -> SDL_FRect &;

    auto init(SDL_Renderer *renderer) -> void;
};
/////////////////////////////////////////////////////////////////////////////////
// Application
/////////////////////////////////////////////////////////////////////////////////
class Application
{

  private:
    bool running{true};

    SDL_Window *window{};
    SDL_Renderer *renderer{};

    FontAtlas font_atlas{"resources/Inter-VariableFont.ttf"};

    SDL_Surface *doc_surface{};
    SDL_Texture *doc_texture{};

    SDL_FRect current_pos{};

    constexpr auto static is_ascii(const SDL_Keycode key) noexcept -> bool;
    constexpr auto static quit(const SDL_Event event) noexcept -> bool;

    auto parse_key(const SDL_Keycode key) noexcept -> void;

  public:
    explicit Application() noexcept = default;
    ~Application()
    {
        SDL_DestroySurface(doc_surface);
        SDL_DestroyTexture(doc_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        SDL_Quit();
    }
    auto init() -> void;
    auto draw() -> void;
    auto draw_ascii_key(const SDL_Keycode key) const noexcept -> void;
    auto run() -> void;
};
