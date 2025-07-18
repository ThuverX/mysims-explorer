#pragma once

#include <optional>

#include "SDL3/SDL_video.h"
#include "SDL3/SDL_events.h"

#include "util/singleton.hpp"
#include "io/AssetMap.hpp"
#include "gfx/Renderer.hpp"
#include "io/Loader.hpp"
#include "Camera.hpp"
#include "io/File.hpp"

#include "essencio/GameType.hpp"

enum class ContextType : uint8_t {
    NONE,
    MODEL,
    MATERIAL,
    MATERIALSET,
    XML,
};

class Context {
    MAKE_SINGLETON(Context)
private:
    SDL_Window *mWindow;
    SDL_GLContext mGLContext;

    GLuint mShaderHandle;
    FramebufferHandle mViewport;

    std::optional<fs::path> mDataRoot;
    essencio::GameType mGameType;
    ContextType mContextType;
    std::optional<AssetMap> mAssetMap;
    DirectoryEntry mRootDirectory;
    Loader mLoader;
    Camera mCamera;

    std::optional<std::string> mCurrentFile;
    std::optional<std::string> mNextFile;

public:
    // View options
    bool mShowExplorer = true;
    bool mShowProperties = true;
    bool mShowConsole = true;

    bool mWireframeMode = false;

    static std::optional<fs::path> FindDataRoot(const fs::path &path);
    static ContextType GetExtensionContextType(const std::string &extension);

    bool Initialize(const std::optional<fs::path> &dataRoot);
    void Update();
    static void ProcessEvent(SDL_Event *event);
    void Render();
    void RenderScene();
    void Shutdown();

    static void Quit();
    
    void ChangeDataRoot(const std::optional<fs::path> &dataRoot);
    void ChangeGameType(const essencio::GameType &gameType);
    void ReloadAssetMap();

    void SetNextFile(const std::optional<std::string> &path);

private:
    void LoadFile(const fs::path &path);

public:
    [[nodiscard]] inline std::optional<fs::path> GetDataRoot() const {
        return mDataRoot;
    }

    [[nodiscard]] inline essencio::GameType GetGameType() const {
        return mGameType;
    }

    [[nodiscard]] inline ContextType GetContextType() const {
        return mContextType;
    }

    [[nodiscard]] inline const std::optional<AssetMap> &GetAssetMap() const {
        return mAssetMap;
    }

    [[nodiscard]] inline const DirectoryEntry &GetRootDirectory() const {
        return mRootDirectory;
    }

    inline Loader &GetLoader() {
        return mLoader;
    }

    inline Camera &GetCamera() {
        return mCamera;
    }

    [[nodiscard]] inline const std::optional<std::string> &GetCurrentFile() const {
        return mCurrentFile;
    }
};
