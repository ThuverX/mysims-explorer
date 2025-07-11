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
#include "essencio/material/kingdom/Material.hpp"

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

void Context::LoadModel(const char *path) {
    // TODO: Move this to somewhere more central
    auto gameRoot = FindGameRoot(fs::path(path));

    if (gameRoot) {
        mGameRoot = gameRoot->string();
    }

    std::vector<uint8_t> file = File::ReadFile(path);
    essencio::BinReader reader(file.data(), file.size());

    essencio::WindowsModel model;
    essencio::WindowsModel::Read(model, reader);

    for (const auto &mesh : model.meshes) {
        std::vector<float> vertices;
        std::vector<uint32_t> indices;

        uint32_t positionOffset = 0xFFFFFFFF;
        uint32_t uvOffset = 0xFFFFFFFF;

        for (const auto& key : mesh.vertexKeys) {
            //if (key.index == 0 && key.type == essencio::VertexKeyType::FLOAT3) {
            if (key.type == essencio::VertexKeyType::FLOAT3 && positionOffset == 0xFFFFFFFF) {
                positionOffset = key.offset;
            } else if (key.type == essencio::VertexKeyType::FLOAT2 && uvOffset == 0xFFFFFFFF) {
                uvOffset = key.offset;
            }
        }

        // TODO: Throw exceptions instead?
        if (positionOffset == 0xFFFFFFFF) {
            std::cerr << "No FLOAT3 position key found in vertexKeys!" << std::endl;
            return;
        }
        if (uvOffset == 0xFFFFFFFF) {
            std::cerr << "No FLOAT2 UV key found in vertexKeys!" << std::endl;
            return;
        }

        size_t stride = mesh.vertexArraySize / mesh.numVertices;
        vertices.reserve(mesh.numVertices * 5); // 3 for pos + 2 for UV

        for (size_t i = 0; i < mesh.numVertices; ++i) {
            size_t base = i * stride;

            float x = *reinterpret_cast<const float*>(&mesh.vertices[base + positionOffset + 0]);
            float y = *reinterpret_cast<const float*>(&mesh.vertices[base + positionOffset + 4]);
            float z = *reinterpret_cast<const float*>(&mesh.vertices[base + positionOffset + 8]);

            float u = *reinterpret_cast<const float*>(&mesh.vertices[base + uvOffset + 0]);
            float v = *reinterpret_cast<const float*>(&mesh.vertices[base + uvOffset + 4]);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(u);
            vertices.push_back(v);
        }

        // Load indices
        indices.reserve(mesh.numFaces * 3);

        for (const auto& face : mesh.faces) {
            indices.push_back(face.a);
            indices.push_back(face.b);
            indices.push_back(face.c);
        }

        MeshHandle meshHandle = Renderer::CreateMesh({
            vertices,
            indices,
        });

        // Load material
        fs::path parentDirectory = fs::path(path).parent_path();
        std::string texturePath = File::GetResourceKeyPath(mesh.material, "Material");

        // Get the final path string
        fs::path materialPath = parentDirectory / texturePath;
        std::cout << materialPath.string() << std::endl;

        std::vector<uint8_t> materialData = File::ReadFile(materialPath.string().c_str());
        essencio::BinReader materialReader(materialData.data(), materialData.size());

        essencio::kingdom::Material material;
        essencio::kingdom::Material::Read(material, materialReader);

        // Read material
        for (const auto &param : material.data.params) {
            switch (param.valueType) {
                case essencio::kingdom::MaterialParameterType::RESOURCE_KEY:
                    {
                        std::string texturePath = File::GetResourceKeyPath(param.mapKey, "dds");
                        texturePath = (mGameRoot / "GameData_Win64/Textures/Objects" / texturePath).string();

                        // Load the DDS texture using gli
                        gli::texture texture = gli::load(texturePath);
                        if (texture.empty()) {
                            std::cerr << "Failed to load texture: " << texturePath << std::endl;
                            return;
                        }

                        gli::gl GL(gli::gl::PROFILE_GL33);
                        gli::gl::format const format = GL.translate(texture.format(), texture.swizzles());
                        GLsizei const levels = static_cast<GLsizei>(texture.levels());

                        GLuint textureID = Renderer::CreateTexture({
                            texture,
                            format,
                            levels,
                        });

                        // just assign to current mesh...
                        meshHandle.textureID = textureID;
                    }
                    break;
                default:
                    // Just do nothing for now
                    break;
            }
        }

        mMeshHandles.emplace_back(meshHandle);
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
        glBindTexture(GL_TEXTURE_2D, handle.textureID);
        glUniform1i(glGetUniformLocation(mShaderHandle, "uTexture"), 0);

        glBindVertexArray(handle.VAO);
        glDrawElements(GL_TRIANGLES, handle.indexCount, GL_UNSIGNED_INT, 0);
    }

    SDL_GL_SwapWindow(mWindow);
}

void Context::Shutdown() {

    for (auto &mesh : mMeshHandles) {
        // Delete texture if available
        // NOTE: This is slighty hacky since technically textures could be loaded
        // that are never getting deleted
        if (mesh.textureID != 0) {
            Renderer::DestroyTexture(mesh.textureID);
        }

        Renderer::DestroyMesh(mesh);
    }

    Renderer::DestroyShader(mShaderHandle);

    SDL_GL_DestroyContext(mGLContext);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}
