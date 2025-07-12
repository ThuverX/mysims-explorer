#include "Context.hpp"

#include <iostream>

#include "Renderer.hpp"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "glad/gl.h"

#include "Context.hpp"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

#include "File.hpp"

#include "SDL3/SDL_init.h"

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <gli/gli.hpp>

#include "essencio/BinReader.hpp"
#include "essencio/model/WindowsModel.hpp"
#include "essencio/material/Material.hpp"

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

bool Context::Initialize() {
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    mWindow = SDL_CreateWindow("MySims Viewer", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
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

    SDL_ShowWindow(mWindow);
    return true;
}

// TODO: Get rid of this function
void Context::LoadModel(const char *path) {
    // TODO: Move this to somewhere more central
    auto gameRoot = FindGameRoot(fs::path(path));

    if (gameRoot) {
        mGameRoot = gameRoot->string();
    }

    auto model = mLoader.LoadModel(path);

    if (model) {
        // Load the mesh handles into the global state for now
        for (const auto &mesh : model->meshes) {
            mMeshHandles.push_back(mesh);
        }
    }
}

void Context::Render() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe

    glUseProgram(mShaderHandle);

    // Animate rotation
    static float time = 0.0f;
    time += 0.01f; // radians per frame

    // Build MVP using glm
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    model = glm::rotate(model, time, glm::vec3(0.f, 1.f, 0.f));

    //glm::mat4 view = glm::mat4(1.0f); // no camera yet
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f), // camera position
        glm::vec3(0.0f, 0.5f, 0.0f), // look at center
        glm::vec3(0.0f, 1.0f, 0.0f)  // up vector
    );

    glm::mat4 mvp = proj * view * model;

    // Upload to shader
    GLint mvpLoc = glGetUniformLocation(mShaderHandle, "uMVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    for (const auto &handle : mMeshHandles) {
        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, handle.texture);
        glUniform1i(glGetUniformLocation(mShaderHandle, "uTexture"), 0);

        glBindVertexArray(handle.VAO);
        glDrawElements(GL_TRIANGLES, handle.indexCount, GL_UNSIGNED_INT, 0);
    }

    SDL_GL_SwapWindow(mWindow);
}

void Context::Shutdown() {

    // TODO: Replace this with Loader unloading
    for (auto &mesh : mMeshHandles) {
        // Delete texture if available
        // NOTE: This is slighty hacky since technically textures could be loaded
        // that are never getting deleted
        if (mesh.texture != 0) {
            Renderer::DestroyTexture(mesh.texture);
        }

        Renderer::DestroyMesh(mesh);
    }

    Renderer::DestroyShader(mShaderHandle);

    SDL_GL_DestroyContext(mGLContext);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}
