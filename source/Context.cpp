#include "Context.hpp"

#include "Renderer.hpp"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "UI.hpp"
#include "glad/gl.h"

#include "Context.hpp"
#include "AssetMap.hpp"
#include "macros/log.hpp"
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

#include "version.h"

#define VERTEX_SHADER_SOURCE "#version 330 core\n" \
    "layout(location = 0) in vec3 aPos;\n" \
    "layout(location = 1) in vec2 aTexCoord;\n" \
    "\n" \
    "out vec2 TexCoord;\n" \
    "\n" \
    "uniform mat4 uMVP;\n" \
    "\n" \
    "void main() {\n" \
    "    gl_Position = uMVP * vec4(aPos, 1.0);\n" \
    "    TexCoord = aTexCoord;\n" \
    "}\n"

#define FRAGMENT_SHADER_SOURCE "#version 330 core\n" \
    "in vec2 TexCoord;\n" \
    "out vec4 FragColor;\n" \
    "\n" \
    "uniform sampler2D uTexture;\n" \
    "\n" \
    "void main() {\n" \
    "    FragColor = texture(uTexture, TexCoord);\n" \
    "}\n"

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

        if (current == current.parent_path())
            return std::nullopt;

        current = current.parent_path();
    }

    return std::nullopt;
}

ContextType Context::GetExtensionContextType(const std::string &extension) {
    if (extension == ".0xb359c791") {
        return ContextType::MODEL;
    } else if (extension == ".Material") {
        return ContextType::MATERIAL;
    } else if (extension == ".xml") {
        return ContextType::XML;
    }
    return ContextType::NONE;
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
    mWindow = SDL_CreateWindow(title.c_str(), 1280, 720, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    if (!mWindow) {
        LOG_ERROR("Failed to create SDL window: %s", SDL_GetError());
        return false;
    }

    mGLContext = SDL_GL_CreateContext(mWindow);
    if (!mGLContext) {
        LOG_ERROR("Failed to create GL context: %s", SDL_GetError());
        return false;
    }

    SDL_GL_MakeCurrent(mWindow, mGLContext);

    int version = gladLoadGL(SDL_GL_GetProcAddress);
    if (version == 0) {
        LOG_ERROR("Failed to initialize OpenGL loader");
        return false;
    }
    
    glEnable(GL_DEPTH_TEST);

    // Load a simple default shader
    mShaderHandle = Renderer::CreateShader({
        VERTEX_SHADER_SOURCE,
        FRAGMENT_SHADER_SOURCE
    });
    // TODO: Add error checking

    // Create main viewport framebuffer (maybe allow multiple later on?)
    mViewport = Renderer::CreateFramebuffer(1280, 720);
    // TODO: Add error checking
    
    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = nullptr;

    // Setup style
    ImGui::StyleColorsDark();
    // Initialize backends
    ImGui_ImplSDL3_InitForOpenGL(mWindow, mGLContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    SDL_ShowWindow(mWindow);

    LOG_SUCCESS("MySims Explorer was successfully initialized!");
    return true;
}

void Context::Update() {
    if (mNextFile) {
        LoadFile(*mNextFile);
        mCurrentFile = mNextFile;
        mNextFile = std::nullopt;
    }
}

void Context::ProcessEvent(SDL_Event *event) {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void Context::Render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    UI::MainMenuBar();
    UI::DockSpace();

    if (mShowExplorer) UI::Explorer();
    if (mShowProperties) UI::Properties();
    if (mShowConsole) UI::Console();

    // ImGui::UpdatePlatformWindows();
    // ImGui::RenderPlatformWindowsDefault();

    // Any 3D scenes that should be displayed are rendered to a framebuffer first
    if (mContextType == ContextType::MODEL) {
        RenderScene();
    }

    UI::Viewport(mContextType, mViewport);
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(mWindow);
}

void Context::RenderScene() {

    glBindFramebuffer(GL_FRAMEBUFFER, mViewport.FBO);
    glViewport(0, 0, mViewport.width, mViewport.height);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (mWireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glUseProgram(mShaderHandle);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = mCamera.GetProjectionMatrix(
        glm::vec2(mViewport.width, mViewport.height));
    glm::mat4 view = mCamera.GetViewMatrix();

    glm::mat4 mvp = projection * view * model;

    // Upload to shader
    GLint mvpLoc = glGetUniformLocation(mShaderHandle, "uMVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    for (const auto &model : mLoader.GetModels()) {
        for (const auto &mesh : model.second.meshes) {
            // Skip invisible meshes
            if (!mesh.isVisible) continue;

            // Bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.handle.texture);
            glUniform1i(glGetUniformLocation(mShaderHandle, "uTexture"), 0);

            glBindVertexArray(mesh.handle.VAO);
            glDrawElements(GL_TRIANGLES, mesh.handle.indexCount, GL_UNSIGNED_INT, 0);
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

    if (mDataRoot == dataRoot)
        return;

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

        LOG_TRACE("Automatically detected game type %d", static_cast<int>(mGameType));
        // Build and cache directory tree
        mRootDirectory = File::BuildDirectoryTree(*mDataRoot);
    }

    ReloadAssetMap();
}

void Context::ChangeGameType(const essencio::GameType &gameType) {

    if (mGameType == gameType)
        return;

    mLoader.UnloadAll();
    mGameType = gameType;

    ReloadAssetMap();

    LOG_INFO("Game type was changed to %d", static_cast<int>(mGameType));
}

void Context::SetNextFile(const std::optional<std::string> &path) {

    if (mCurrentFile == path)
        return;

    mNextFile = path;
}

void Context::ReloadAssetMap() {

    auto dataRoot = GetDataRoot();
    if (!dataRoot) return;

    fs::path assetMapPath = fs::path(*dataRoot) / "BuildData" / "AssetGenerator" / "AssetMap.xml";
    mAssetMap = AssetMap::Read(assetMapPath.string());
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
        case ContextType::XML:
            mLoader.LoadXml(path.string());
            break;
        default:
            // Nothing to load here...
            break;
    }
}
