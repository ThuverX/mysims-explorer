#pragma once

#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_events.h"

#include "macros/singleton.hpp"
#include "Renderer.hpp"
#include "Loader.hpp"
#include "Camera.hpp"

#include "essencio/GameType.hpp"

enum class ViewportType {
    NONE,
    MODEL,
    MATERIAL
};

class Context {
    MAKE_SINGLETON(Context)
private:
    SDL_Window *mWindow;
    SDL_GLContext mGLContext;

    GLuint mShaderHandle;
    FramebufferHandle mViewport;

    std::optional<fs::path> mGameRoot;
    essencio::GameType mGameType;
    ViewportType mViewportType;
    Loader mLoader;
    Camera mCamera;

public:
    // View options
    bool mShowExplorer = true;
    bool mShowProperties = true;
    bool mShowConsole = true;

    bool mWireframeMode = false;

    static std::optional<fs::path> FindGameRoot(const fs::path &path);

    bool Initialize(const std::optional<fs::path> &gameRoot);
    void Update();
    void ProcessEvent(SDL_Event *event);
    void Render();
    
    void RenderLoadedModel();
    void RenderLoadedMaterial();

    void Shutdown();

    void ChangeGameRoot(const std::optional<fs::path> &gameRoot);
    void ChangeGameType(const essencio::GameType &gameType);

    void LoadViewportFile(const fs::path &path);

    inline std::optional<fs::path> GetGameRoot() const {
        return mGameRoot;
    }

    inline essencio::GameType GetGameType() const {
        return mGameType;
    }

    inline ViewportType GetViewportType() const {
        return mViewportType;
    }

    inline Loader &GetLoader() {
        return mLoader;
    }

    inline Camera &GetCamera() {
        return mCamera;
    }
};
