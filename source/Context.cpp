#include "Context.hpp"

#include <iostream>

#include "Renderer.hpp"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "UI.hpp"
#include "glad/gl.h"

#include "Context.hpp"
#include <iostream>
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

std::optional<fs::path> Context::FindGameRoot(const fs::path &path) {
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

        current = current.parent_path();
    }

    return std::nullopt;
}

bool Context::Initialize(const std::optional<fs::path> &gameRoot) {

    mGameRoot = gameRoot;
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    mWindow = SDL_CreateWindow("MySims Explorer", 1280, 720, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    if (!mWindow) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        return false;
    }

    mGLContext = SDL_GL_CreateContext(mWindow);
    if (!mGLContext) {
        std::cerr << "Failed to create GL context: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_MakeCurrent(mWindow, mGLContext);

    int version = gladLoadGL(SDL_GL_GetProcAddress);
    if (version == 0) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        return false;
    }
    
    glEnable(GL_DEPTH_TEST);

    // Load a simple default shader
    mShaderHandle = Renderer::CreateShader({
        VERTEX_SHADER_SOURCE,
        FRAGMENT_SHADER_SOURCE
    });

    // Create main viewport framebuffer (maybe allow multiple later on?)
    mViewport = Renderer::CreateFramebuffer(1280, 720);
    
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
    return true;
}

void Context::Update() {
    // TODO
}

void Context::ProcessEvent(SDL_Event *event) {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void Context::Render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    UI::DrawDockSpace();
    UI::DrawFileExplorer();
    //UI::DrawProperties();
    //UI::DrawConsole();
    UI::DrawMainMenuBar();

    // ImGui::UpdatePlatformWindows();
    // ImGui::RenderPlatformWindowsDefault();

    glBindFramebuffer(GL_FRAMEBUFFER, mViewport.FBO);
    glViewport(0, 0, mViewport.width, mViewport.height);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe

    glUseProgram(mShaderHandle);

    // Build MVP using glm
    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 projection = mCamera.GetProjectionMatrix(glm::vec2(mViewport.width, mViewport.height));
    glm::mat4 view = mCamera.GetViewMatrix();

    glm::mat4 mvp = projection * view * model;

    // Upload to shader
    GLint mvpLoc = glGetUniformLocation(mShaderHandle, "uMVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    for (const auto &model : mLoader.GetModels()) {
        for (const auto &mesh : model.second.meshes) {
            // Bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.texture);
            glUniform1i(glGetUniformLocation(mShaderHandle, "uTexture"), 0);

            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    UI::DrawViewport(mViewport);

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(mWindow);
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

void Context::ChangeGameRoot(const std::optional<fs::path> &gameRoot) {

    if (mGameRoot == gameRoot)
        return;

    mLoader.UnloadAll();
    mGameRoot = gameRoot;

    if (mGameRoot) {
        // Try to determine game type
        if (fs::exists(*mGameRoot / "GameData" / "Vaults")) {
            mGameType = essencio::GameType::KINGDOM;
        } else {
            mGameType = essencio::GameType::MYSIMS;
        }
    }
}

void Context::ChangeGameType(const essencio::GameType &gameType) {

    if (mGameType == gameType)
        return;

    mLoader.UnloadAll();
    mGameType = gameType;
}
