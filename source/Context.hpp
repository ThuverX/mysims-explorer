#pragma once

#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

#include "SDL3/SDL_video.h"
#include "Renderer.hpp"
#include "Loader.hpp"

class Context {
private:
    SDL_Window *mWindow;
    SDL_GLContext mGLContext;

    GLuint mShaderHandle;
    std::vector<MeshHandle> mMeshHandles;
    fs::path mGameRoot;

    Loader mLoader;

    Context() = default;
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context &operator=(const Context &) = delete;
    Context &operator=(Context &&) = delete;

    static std::optional<fs::path> FindGameRoot(const fs::path &path);

public:
    bool Initialize();
    void LoadModel(const char *path);
    void Render();
    void Shutdown();

    inline fs::path GetGameRoot() const {
        return mGameRoot;
    }

    static Context &Get() {
        static Context instance;
        return instance;
    }
};
