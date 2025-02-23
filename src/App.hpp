#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <string>
#include <vector>

class Application
{
    template <typename T> using Vec = std::vector<T>;

  private:
    bool running{true};
    const std::string text{"Hello World!"};

    SDL_Window *window{};
    SDL_Renderer *renderer{};
    TTF_Font *font{};
    Vec<SDL_Surface *> surfaces{};
    SDL_Texture *finished_texture{};
    SDL_FRect rectangle{};

    constexpr auto static is_digit(const SDL_Event &event) noexcept -> bool;
    constexpr auto static is_letter(const SDL_Event &event) noexcept -> bool;
    constexpr auto static is_ascii(const SDL_Event &event) noexcept -> bool;
    constexpr auto static quit(const SDL_Event &event) noexcept -> bool;

    auto clear_screen() const noexcept -> void;

  public:
    explicit Application() = default;
    ~Application()
    {
        SDL_DestroyTexture(finished_texture);
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        TTF_Quit();
        SDL_Quit();
    }
    auto init() -> void;
    auto render() -> void;
    auto draw() const noexcept -> void;
    auto run() -> void;
};
