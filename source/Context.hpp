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
#include "ui/UIState.hpp"

#include "essencio/GameType.hpp"

#if defined(_WIN32) || defined(_WIN64)
    // Default Steam paths
    static const char *KINGDOM_STEAM_PATH = R"(C:\Program Files (x86)\Steam\steamapps\common\MySims Kingdom\data)";
    static const char *MYSIMS_STEAM_PATH = R"(C:\Program Files (x86)\Steam\steamapps\common\MySims\data)";
    // Default EA App paths
    static const char *KINGDOM_EA_PATH = R"(C:\Program Files\EA Games\MYSIMS KINGDOM)";
    static const char *MYSIMS_EA_PATH = R"(C:\Program Files\EA Games\MYSIMS)";
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
public:
    static constexpr int DEFAULT_VIEWPORT_WIDTH = 1280;
    static constexpr int DEFAULT_VIEWPORT_HEIGHT = 720;

    // Static helper functions
    static std::optional<fs::path> FindDataRoot(const fs::path &path);
    static ContextType GetExtensionContextType(const std::string &extension);

    // Static flow functions
    static void ProcessEvent(SDL_Event *event);    
    static void Quit();

private:
    // Platform resources
    SDL_Window *mWindow;
    SDL_GLContext mGLContext;

    // Graphics resources
    ShaderHandle mShaderHandle;
    MeshHandle mCubeMesh;
    TextureHandle mCubeTexture;
    FramebufferHandle mViewport;

    std::optional<fs::path> mDataRoot;
    essencio::GameType mGameType;
    ContextType mContextType;

    std::optional<AssetMap> mAssetMap;
    FileEntry mRootDirectory;
    // Keep track of the previous theme state to update it correctly
    bool mIsDarkTheme;
    UIState mUIState;
    Loader mLoader;
    Camera mCamera;

    std::optional<std::string> mCurrentFile; // The currently loaded filename
    std::optional<std::string> mEnqueuedFile; // The next file to load after this frame

public:
    bool Initialize(const std::optional<fs::path> &dataRoot);
    void Update();
    
    void Render();
    void RenderScene();
    void Shutdown();

    void ChangeDataRoot(const std::optional<fs::path> &dataRoot);
    void ChangeGameType(const essencio::GameType &gameType);
    void ReloadAssetMap();

    // Set the next file to be loaded
    void SetEnqueuedFile(const std::optional<std::string> &path);

private:
    bool InitializeResources();
    bool InitializeImGui();

    // Applies the current search query to all file entries recursively
    static bool ApplySearchQuery(FileEntry& entry, const char* query);

    void LoadFile(const fs::path &path);

public:
    [[nodiscard]] inline std::optional<fs::path> GetDataRoot() const { return mDataRoot; }
    [[nodiscard]] inline essencio::GameType GetGameType() const { return mGameType; }
    [[nodiscard]] inline ContextType GetContextType() const { return mContextType; }

    [[nodiscard]] inline const std::optional<AssetMap> &GetAssetMap() const { return mAssetMap; }
    [[nodiscard]] inline const FileEntry &GetRootDirectory() const { return mRootDirectory; }
    [[nodiscard]] inline const UIState &GetUIState() const { return mUIState; }

    inline Loader &GetLoader() { return mLoader; }
    inline Camera &GetCamera() { return mCamera; }

    [[nodiscard]] inline const std::optional<std::string> &GetCurrentFile() const {
        return mCurrentFile;
    }    
};
