#pragma once

#include <SDL3/SDL_stdinc.h>
#include <glad/gl.h>
#include <vector>

// DDS texture loading
#include "gli/gl.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "essencio/GameType.hpp"
#include "gli/texture.hpp"

struct MeshData;

using ShaderHandle = GLuint;
using TextureHandle = GLuint;

struct ShaderCreateInfo {
    const char *vertexShader;
    const char *fragmentShader;
};

struct TextureCreateInfo {
    gli::texture texture;
    gli::gl::format format;
    GLsizei levels;
};

struct MeshHandle {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    size_t indexCount;
    std::vector<TextureHandle> textures;
};

struct MeshCreateInfo {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
};

struct FramebufferHandle {
    GLuint FBO;
    TextureHandle texture;
    GLuint depth;
    int width;
    int height;
};

namespace Renderer {

static constexpr uint32_t LOG_INFO_SIZE = 512;

bool Initialize(GLADloadfunc loader);

ShaderHandle CreateShader(const ShaderCreateInfo &info);
void DestroyShader(ShaderHandle &shader);

TextureHandle CreateTexture(const TextureCreateInfo &info);
// Creates a single pixel texture of the specified color to use as a placeholder
TextureHandle CreateColorTexture(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void DestroyTexture(TextureHandle &texture);

MeshHandle CreateMesh(const MeshCreateInfo &info);
void DrawMesh(const MeshData&mesh, const glm::mat4 &mvp, const essencio::GameType &gameType, bool drawLines = false);
void DestroyMesh(const MeshData &mesh);

FramebufferHandle CreateFramebuffer(int width, int height);
void DestroyFramebuffer(const FramebufferHandle &framebuffer);

};
