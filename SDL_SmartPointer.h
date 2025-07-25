#pragma once
#include <memory>
#include <SDL2/SDL.h>
#include <iostream>

template <typename T>
struct SDL_Deleter {
    void operator()(T* ptr) const {
        static_assert(sizeof(T) == 0, "SDL_Deleter not specialized for this type");
    }
};

template <>
struct SDL_Deleter<SDL_Texture> {
    void operator()(SDL_Texture* ptr) const {
        if (ptr) {
            SDL_DestroyTexture(ptr);
            std::cout << "SDL_Texture deleted" << std::endl;
        }
    }
};

template <>
struct SDL_Deleter<SDL_Surface> {
    void operator()(SDL_Surface* ptr) const {
        if (ptr) {
            SDL_FreeSurface(ptr);
            std::cout << "SDL_Surface deleted" << std::endl;
        }
    }
};

template <typename T>
class SDL_SmartPointer {
public:
    // Constructor
    explicit SDL_SmartPointer(T* rawPtr = nullptr)
        : ptr_(rawPtr) {}

    SDL_SmartPointer(const SDL_SmartPointer&) = delete;
    SDL_SmartPointer& operator=(const SDL_SmartPointer&) = delete;
    SDL_SmartPointer(SDL_SmartPointer&&) noexcept = default;
    SDL_SmartPointer& operator=(SDL_SmartPointer&&) noexcept = default;
    std::unique_ptr<T, SDL_Deleter<T>> ptr_;
};