#include <iostream>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "Context.hpp"

SDL_AppResult SDL_AppInit([[maybe_unused]] void **appstate, int argc, char **argv) {
    // TODO: Just show empty UI when no model is specified through the command-line
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return SDL_APP_SUCCESS;
    }

    auto gameRoot = Context::FindGameRoot(argv[1]);

    if (!Context::Get().Initialize(gameRoot)) {
        return SDL_APP_FAILURE;
    }

    Context::Get().GetLoader().LoadModel(argv[1]);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate([[maybe_unused]] void *appstate) {
    Context::Get().Render();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent([[maybe_unused]] void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit([[maybe_unused]] void *appstate, [[maybe_unused]] SDL_AppResult result) {
    Context::Get().Shutdown();
}
