#pragma once

#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_events.h"

#include "Renderer.hpp"
#include "Loader.hpp"

class Context {
private:
    SDL_Window *mWindow;
    SDL_GLContext mGLContext;

    GLuint mShaderHandle;
    FramebufferHandle mViewport;

    std::optional<fs::path> mGameRoot;
    Loader mLoader;

    Context() = default;
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context &operator=(const Context &) = delete;
    Context &operator=(Context &&) = delete;

public:
    static std::optional<fs::path> FindGameRoot(const fs::path &path);

    bool Initialize(const std::optional<fs::path> &gameRoot);
    void Update();
    void ProcessEvent(SDL_Event *event);
    void Render();
    void Shutdown();

    inline std::optional<fs::path> GetGameRoot() const {
        return mGameRoot;
    }

    inline void SetGameRoot(const std::optional<fs::path> value) {
        // TODO: Actually reload the current viewer state
        mGameRoot = value;
    }

    inline Loader &GetLoader() {
        return mLoader;
    }

    static Context &Get() {
        static Context instance;
        return instance;
    }
};
