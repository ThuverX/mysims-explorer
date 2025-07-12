#pragma once

#include <glad/gl.h>
#include <vector>

// DDS texture loading
#include "gli/gl.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "gli/texture.hpp"

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
    GLsizei indexCount;
    TextureHandle texture;
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

ShaderHandle CreateShader(const ShaderCreateInfo &info);
void DestroyShader(ShaderHandle &shader);

TextureHandle CreateTexture(const TextureCreateInfo &info);
void DestroyTexture(TextureHandle &handle);

MeshHandle CreateMesh(const MeshCreateInfo &info);
void DestroyMesh(MeshHandle &mesh);

FramebufferHandle CreateFramebuffer(int width, int height);
void DestroyFramebuffer(const FramebufferHandle &framebuffer);

};
