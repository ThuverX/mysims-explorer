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

#if defined(_WIN32) || defined(_WIN64)
    static const char *KINGDOM_STEAM_PATH = R"(C:\Program Files (x86)\Steam\steamapps\common\MySims Kingdom\data)";
    static const char *MYSIMS_STEAM_PATH = R"(C:\Program Files (x86)\Steam\steamapps\common\MySims\data)";
#endif

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
    MeshHandle mCubeMesh;
    TextureHandle mCubeTexture;
    FramebufferHandle mViewport;

    std::optional<fs::path> mDataRoot;
    essencio::GameType mGameType;
    ContextType mContextType;
    std::optional<AssetMap> mAssetMap;
    FileEntry mRootDirectory;
    Loader mLoader;
    Camera mCamera;

    std::optional<std::string> mCurrentFile;
    std::optional<std::string> mNextFile;
    
    // Theme options
    bool mIsDarkTheme = true;

public:
    // TODO: Move this section to some separate UI state
    // View options
    bool mShowExplorer = true;
    bool mShowProperties = true;
    bool mShowConsole = true;

    // Rendering options
    bool mWireframeMode = false;
    bool mShowBounds = false;

    char mSearchQuery[256];

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
    void SetIsDarkTheme(bool isDarkTheme);

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

    [[nodiscard]] inline const FileEntry &GetRootDirectory() const {
        return mRootDirectory;
    }

    inline Loader &GetLoader() {
        return mLoader;
    }

    inline Camera &GetCamera() {
        return mCamera;
    }

    [[nodiscard]] inline bool GetIsDarkTheme() const { return mIsDarkTheme; }

    [[nodiscard]] inline const std::optional<std::string> &GetCurrentFile() const {
        return mCurrentFile;
    }
};
