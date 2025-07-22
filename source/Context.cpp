#include "Context.hpp"

#include "essencio/GameType.hpp"
#include "gfx/Renderer.hpp"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "io/File.hpp"
#include "ui/UI.hpp"
#include "glad/gl.h"

#include "io/AssetMap.hpp"
#include "ui/Properties.hpp"
#include "ui/Explorer.hpp"
#include "ui/Viewport.hpp"
#include "ui/Console.hpp"
#include "util/log.hpp"
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_timer.h>
#include <filesystem>

namespace fs = std::filesystem;

#include "SDL3/SDL_init.h"

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <gli/gli.hpp>

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"

#include "IconsLucide.h"
#include "data/lucide.hpp"

#include "version.h"

#include <algorithm> // for std::transform
#include <string>

#include "data/shaders.hpp"
#include "data/cube.hpp"

std::optional<fs::path> Context::FindDataRoot(const fs::path &path) {
    fs::path current = path;

    // If the input path is a file, go to its parent
    if (fs::is_regular_file(current)) {
        current = current.parent_path();
    }

    while (!current.empty()) {
        fs::path gameData = current / "GameData";
        fs::path gameDataWin64 = current / "GameData_Win64";

        if (fs::exists(gameData) && fs::is_directory(gameData)) {
            return current;
        }
        if (fs::exists(gameDataWin64) && fs::is_directory(gameDataWin64)) {
            return current;
        }

        if (current == current.parent_path()) {
            return std::nullopt;
        }

        current = current.parent_path();
    }

    return std::nullopt;
}

ContextType Context::GetExtensionContextType(const std::string &extension) {
    if (extension == ".0xb359c791") {
        return ContextType::MODEL;
    } else if (extension == ".Material") {
        return ContextType::MATERIAL;
    } else if (extension == ".MaterialSet") {
        return ContextType::MATERIALSET;
    } else if (extension == ".xml" || extension == ".CABXml") {
        return ContextType::XML;
    }
    return ContextType::NONE;
}

std::string Context::GetFileEntryDisplayName(FileEntry &entry, const AssetMap &assetMap) {
    
    if (entry.displayName != std::nullopt) {
        return *entry.displayName;
    }

    std::string displayName = entry.name;
    // Currently, we're only translating asset map names for Kingdom
    // MySims seems to have some weirdness going on in terms of uniqueness
    if (Context::Get().GetGameType() == essencio::GameType::KINGDOM) {
        auto mapping = assetMap.Get(entry.path.stem().string());
        if (mapping) {
            displayName = *mapping + entry.path.extension().string();
        }
    }

    entry.displayName = displayName;
    return displayName;
}

bool Context::Initialize(const std::optional<fs::path> &dataRoot) {
    LOG_INFO("MySims Explorer v%s is initializing...", VERSION_STRING);
    ChangeDataRoot(dataRoot);
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_ERROR("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    std::string title = "MySims Explorer v" + std::string(VERSION_STRING);
    mWindow = SDL_CreateWindow(title.c_str(), DEFAULT_VIEWPORT_WIDTH, DEFAULT_VIEWPORT_HEIGHT, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    if (mWindow == nullptr) {
        LOG_ERROR("Failed to create SDL window: %s", SDL_GetError());
        return false;
    }

    mGLContext = SDL_GL_CreateContext(mWindow);
    if (mGLContext == nullptr) {
        LOG_ERROR("Failed to create GL context: %s", SDL_GetError());
        return false;
    }

    SDL_GL_MakeCurrent(mWindow, mGLContext);
    Renderer::Initialize(SDL_GL_GetProcAddress);

    // Initialize GPU resources such as viewport, shaders, etc.
    if (!InitializeResources()) return false;
    // Initialize ImGui, including custom icon fonts, etc.
    if (!InitializeImGui()) return false;

    // Setup initial style
    mIsDarkTheme = SDL_GetSystemTheme() != SDL_SYSTEM_THEME_LIGHT;
    mUIState.mIsDarkTheme = mIsDarkTheme;

    if (mUIState.mIsDarkTheme) {
        ImGui::StyleColorsDark();
    } else {
        ImGui::StyleColorsLight();
    }

    SDL_ShowWindow(mWindow);

    LOG_INFO("MySims Explorer was successfully initialized!");
    return true;
}

bool Context::InitializeResources() {
    // Load a simple default shader
    mShaderHandle = Renderer::CreateShader({
        VERTEX_SHADER_SOURCE,
        FRAGMENT_SHADER_SOURCE
    });
    // TODO: Add error checking

    // Create a cube mesh which can be used to display bounds in 3D space
    mCubeMesh = Renderer::CreateMesh({
        CUBE_VERTICES,
        CUBE_INDICES
    });
    // TODO: Add error checking
    mCubeTexture = Renderer::CreateColorTexture(255, 255, 255, 255);
    // TODO: Add error checking

    // Main viewport buffer
    mViewport = Renderer::CreateFramebuffer(DEFAULT_VIEWPORT_WIDTH, DEFAULT_VIEWPORT_HEIGHT);
    // TODO: Add error checking

    return true;
}

bool Context::InitializeImGui() {
    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io; // NOLINT(readability-identifier-length)
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = nullptr;

    {
        // Set up default font and icon font
        io.Fonts->AddFontDefault();
        float baseFontSize = 14.0f;
        //float iconFontSize = baseFontSize * 2.0f / 2.5f;
        float iconFontSize = baseFontSize;

        static const ImWchar iconsRanges[] = { ICON_MIN_LC, ICON_MAX_16_LC, 0 };
        ImFontConfig iconsConfig;
        iconsConfig.MergeMode = true;
        //iconsConfig.PixelSnapH = true;
        iconsConfig.GlyphOffset.y = 3.0f;
        iconsConfig.GlyphExtraAdvanceX = 4.0f;
        iconsConfig.OversampleH = 4;
        iconsConfig.OversampleV = 4;
        // Prevent ImGui from freeing our memory
        iconsConfig.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF(LUCIDE_TTF, LUCIDE_TTF_LEN, iconFontSize, &iconsConfig, iconsRanges);
    }
    
    // Initialize backends
    ImGui_ImplSDL3_InitForOpenGL(mWindow, mGLContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

void Context::Update() {
    if (mEnqueuedFile) {
        LoadFile(*mEnqueuedFile);
        mCurrentFile = mEnqueuedFile;
        mEnqueuedFile = std::nullopt;
    }

    static std::string lastSearchQuery;
    static Uint32 lastChangeTime = 0;
    static bool visibilityUpdated = false;

    Uint32 currentTime = SDL_GetTicks();

    // Detect if search query changed
    if (strcmp(mUIState.mSearchQuery, lastSearchQuery.c_str()) != 0) {
        lastSearchQuery = mUIState.mSearchQuery;
        lastChangeTime = currentTime;
        visibilityUpdated = false;
    }

    // Only update visibility if 250 ms have passed since last input
    if (!visibilityUpdated && (currentTime - lastChangeTime) > 250) {
        ApplySearchQuery(mRootDirectory, mUIState.mSearchQuery);
        visibilityUpdated = true;
    }

    if (mIsDarkTheme != mUIState.mIsDarkTheme) {
        if (mUIState.mIsDarkTheme) {
            ImGui::StyleColorsDark();
        } else {
            ImGui::StyleColorsLight();
        }
        mIsDarkTheme = mUIState.mIsDarkTheme;
    }
}

void Context::ProcessEvent(SDL_Event *event) {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void Context::Render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    UI::DrawMainMenuBar(mUIState);
    UI::SetupDockSpace();
    
    if (mUIState.mShowExplorer) UI::Explorer::Draw(mUIState);
    if (mUIState.mShowProperties) UI::Properties::Draw();
    if (mUIState.mShowConsole) UI::Console::Draw(mUIState);

    // Any 3D scenes that should be displayed are rendered to a framebuffer first
    if (mContextType == ContextType::MODEL) {
        RenderScene();
    }

    UI::Viewport::Draw(mContextType, mViewport);
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(mWindow);
}

void Context::RenderScene() {

    glBindFramebuffer(GL_FRAMEBUFFER, mViewport.FBO);
    glViewport(0, 0, mViewport.width, mViewport.height);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (mUIState.mWireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glUseProgram(mShaderHandle);

    glm::mat4 projection = Camera::GetProjectionMatrix(
        glm::vec2(mViewport.width, mViewport.height));
    glm::mat4 view = mCamera.GetViewMatrix();

    glm::mat4 mvp = projection * view * glm::mat4(1.0f);

    for (const auto &model : mLoader.GetModels()) {
        for (const auto &mesh : model.second.meshes) {
            // Skip invisible meshes
            if (!mesh.isVisible) {
                continue;
            }

            if (mesh.handle.textures.size() > 0 && mesh.materialIndex < mesh.handle.textures.size()) {
                TextureHandle texture = mesh.handle.textures[mesh.materialIndex];
                Renderer::DrawMesh(mesh.handle, mShaderHandle, mvp, texture);
            } else {
                Renderer::DrawMesh(mesh.handle, mShaderHandle, mvp);
            }

            if (mUIState.mShowBounds) {
                auto boundsModel = glm::mat4(1.0f);
                boundsModel = glm::translate(boundsModel, mesh.boundsCenter);
                boundsModel = glm::scale(boundsModel, mesh.boundsSize);
                Renderer::DrawMesh(mCubeMesh, mShaderHandle, projection * view * boundsModel, mCubeTexture, true);
            }
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Context::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    mLoader.UnloadAll();
    Renderer::DestroyShader(mShaderHandle);

    SDL_GL_DestroyContext(mGLContext);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

void Context::Quit() {
    SDL_Event event;
    event.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&event);
}

void Context::ChangeDataRoot(const std::optional<fs::path> &dataRoot) {

    if (mDataRoot == dataRoot) {
        return;
    }

    mLoader.UnloadAll();
    mDataRoot = dataRoot;

    LOG_TRACE("Data root was changed to %s", (*dataRoot).string().c_str());

    if (mDataRoot) {
        // Try to determine game type
        if (fs::exists(*mDataRoot / "GameData" / "Vaults")) {
            mGameType = essencio::GameType::KINGDOM;
        } else {
            mGameType = essencio::GameType::MYSIMS;
        }

        std::string gameType;
        switch (mGameType) {
            case essencio::GameType::KINGDOM: gameType = "MySims Kingdom"; break;
            case essencio::GameType::MYSIMS: gameType = "MySims"; break;
        }

        LOG_TRACE("Automatically detected game type: %s", gameType.c_str());
        // Build and cache directory tree
        mRootDirectory = File::BuildFileTree(*mDataRoot);
    }

    ReloadAssetMap();
}

void Context::ChangeGameType(const essencio::GameType &gameType) {
    if (mGameType == gameType) {
        return;
    }

    mLoader.UnloadAll();
    mGameType = gameType;

    ReloadAssetMap();

    LOG_INFO("Game type was changed to %d", static_cast<int>(mGameType));
}

void Context::SetEnqueuedFile(const std::optional<std::string> &path) {
    if (mCurrentFile == path) {
        return;
    }

    mEnqueuedFile = path;
}

void Context::ReloadAssetMap() {

    auto dataRoot = GetDataRoot();
    if (!dataRoot) {
        return;
    }

    fs::path assetMapPath = fs::path(*dataRoot) / "BuildData" / "AssetGenerator" / "AssetMap.xml";
    mAssetMap = AssetMap::Read(assetMapPath.string());
}

// Utility function to do case-insensitive substring check
static bool StringContainsCaseInsensitive(const std::string& str, const std::string& query) {
    std::string lowerStr = str;
    std::string lowerQuery = query;

    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    return lowerStr.find(lowerQuery) != std::string::npos;
}

// Recursively update visibility based on the current search query
bool Context::ApplySearchQuery(FileEntry& entry, const char* query) {
    if (query[0] == '\0') {
        // No filtering, everything visible
        entry.isVisible = true;
        for (auto& child : entry.children) {
            ApplySearchQuery(child, query);
        }
        return true;
    }

    std::string displayName = entry.name;
    if (!entry.isDirectory) {
        const auto& assetMap = Context::Get().GetAssetMap();
        displayName = entry.name;
        if (assetMap) {
            displayName = GetFileEntryDisplayName(entry, *assetMap);
        }
    }

    bool matchesSelf = StringContainsCaseInsensitive(entry.name, query) ||
        StringContainsCaseInsensitive(displayName, query);

    bool anyChildVisible = false;
    if (entry.isDirectory) {
        for (auto& child : entry.children) {
            if (ApplySearchQuery(child, query)) {
                anyChildVisible = true;
            }
        }
    }

    entry.isVisible = matchesSelf || anyChildVisible;
    return entry.isVisible;
}

void Context::LoadFile(const fs::path &path) {
    mLoader.UnloadAll();
    mCamera.Reset();

    mContextType = GetExtensionContextType(path.extension().string());

    switch (mContextType) {
        case ContextType::MODEL:
            mLoader.LoadModel(path.string(), mGameType);
            break;
        case ContextType::MATERIAL:
            mLoader.LoadMaterial(path.string(), mGameType);
            break;
        case ContextType::MATERIALSET:
            mLoader.LoadMaterialSet(path.string(), mGameType);
            break;
        case ContextType::XML:
            mLoader.LoadXml(path.string());
            break;
        default:
            // Nothing to load here...
            break;
    }
}
