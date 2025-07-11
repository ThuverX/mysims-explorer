#pragma once

#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

#include "SDL3/SDL_video.h"
#include "Renderer.hpp"

class Context {
private:
    SDL_Window *mWindow;
    SDL_GLContext mGLContext;

    GLuint mShaderHandle;
    std::vector<MeshHandle> mMeshHandles;
    fs::path mGameRoot;

    Context() = default;
    Context(const Context&) = delete;
    Context(Context&&) = delete;

    static std::optional<fs::path> FindGameRoot(const fs::path &path);

public:
    bool Initialize();
    void LoadModel(const char *path);
    void Render();
    void Shutdown();

    static Context &Get() {
        static Context instance;
        return instance;
    }
};
