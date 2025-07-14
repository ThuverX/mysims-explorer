#include "macros/log.hpp"

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "Context.hpp"

SDL_AppResult SDL_AppInit([[maybe_unused]] void **appstate, int argc, char **argv) {

    if (argc > 1) {
        auto dataRoot = Context::FindDataRoot(argv[1]);

        if (!dataRoot) {
            LOG_ERROR("Failed to find game root at %s", argv[1]);
            return SDL_APP_FAILURE;
        }

        if (!Context::Get().Initialize(dataRoot)) {
            return SDL_APP_FAILURE;
        }
    } else {
        if (!Context::Get().Initialize(std::nullopt)) {
            return SDL_APP_FAILURE;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate([[maybe_unused]] void *appstate) {
    Context::Get().Update();
    Context::Get().Render();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent([[maybe_unused]] void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_FAILURE;
    }

    Context::Get().ProcessEvent(event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit([[maybe_unused]] void *appstate, [[maybe_unused]] SDL_AppResult result) {
    Context::Get().Shutdown();
}
