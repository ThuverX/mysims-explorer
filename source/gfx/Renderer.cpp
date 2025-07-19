#include "Renderer.hpp"

#include "util/log.hpp"
#include <glm/gtc/type_ptr.hpp>

GLuint Renderer::CreateShader(const ShaderCreateInfo &info) {
    // Compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &info.vertexShader, nullptr);
    glCompileShader(vertexShader);

    // Check vertex shader compile status
    int success;
    char infoLog[LOG_INFO_SIZE];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        glGetShaderInfoLog(vertexShader, LOG_INFO_SIZE, nullptr, infoLog);
        LOG_ERROR("Vertex shader compilation failed\n%s", infoLog);
        return 0;
    }

    // Compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &info.fragmentShader, nullptr);
    glCompileShader(fragmentShader);

    // Check fragment shader compile status
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        glGetShaderInfoLog(fragmentShader, LOG_INFO_SIZE, nullptr, infoLog);
        LOG_ERROR("Fragment shader compilation failed\n%s", infoLog);
        return 0;
    }

    // Link shaders into a program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check linking status
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (success == 0) {
        glGetProgramInfoLog(shaderProgram, LOG_INFO_SIZE, nullptr, infoLog);
        LOG_ERROR("Shader program linking failed\n%s", infoLog);
        return 0;
    }

    // Cleanup shaders as they're now in the program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void Renderer::DestroyShader(ShaderHandle &shader) {
    if (shader != 0) {
        glDeleteProgram(shader);
    }
}

GLuint Renderer::CreateTexture(const TextureCreateInfo &info) {
    GLuint textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    for (std::size_t level = 0; level < info.texture.levels(); ++level) {
        glm::tvec3<GLsizei> levelExtent = info.texture.extent(level);
        glCompressedTexImage2D(GL_TEXTURE_2D,
            static_cast<GLint>(level),
            info.format.Internal,
            levelExtent.x,
            levelExtent.y,
            0,
            static_cast<GLsizei>(info.texture.size(level)),
            info.texture.data(0, 0, level));
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, info.levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

TextureHandle Renderer::CreateColorTexture(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    TextureHandle texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    unsigned char pixel[] = { r, g, b, a };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // no mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return texture;
}

void Renderer::DestroyTexture(TextureHandle &texture) {
    if (texture != 0) {
        glDeleteTextures(1, &texture);
    }
}

MeshHandle Renderer::CreateMesh(const MeshCreateInfo &info) {
    MeshHandle mesh;

    mesh.textures.clear();
    mesh.indexCount = info.indices.size();
    
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    // NOLINTNEXTLINE(bugprone-narrowing-conversions)
    glBufferData(GL_ARRAY_BUFFER, info.vertices.size() * sizeof(float), info.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    // NOLINTNEXTLINE(bugprone-narrowing-conversions)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, info.indices.size() * sizeof(uint32_t), info.indices.data(), GL_STATIC_DRAW);

    // position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texcoord attribute (location = 1)
    // NOLINTNEXTLINE(performance-no-int-to-ptr)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return mesh;
}

void Renderer::DrawMesh(const MeshHandle &mesh, const ShaderHandle &shader, const glm::mat4 mvp, const TextureHandle &texture, bool drawLines) {
    GLint mvpLoc = glGetUniformLocation(shader, "uMVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    if (texture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shader, "uTexture"), 0);
    }

    glBindVertexArray(mesh.VAO);
    glDrawElements(drawLines ? GL_LINES : GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
}

void Renderer::DestroyMesh(MeshHandle &mesh) {
    glDeleteBuffers(1, &mesh.EBO);
    glDeleteBuffers(1, &mesh.VBO);
    glDeleteVertexArrays(1, &mesh.VAO);
}

FramebufferHandle Renderer::CreateFramebuffer(int width, int height) {
    FramebufferHandle framebuffer;
    framebuffer.width = width;
    framebuffer.height = height;

    glGenFramebuffers(1, &framebuffer.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.FBO);

    // Create color texture
    glGenTextures(1, &framebuffer.texture);
    glBindTexture(GL_TEXTURE_2D, framebuffer.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.texture, 0);

    // Create depth renderbuffer
    glGenRenderbuffers(1, &framebuffer.depth);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,framebuffer.depth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer is incomplete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return framebuffer;
}

void Renderer::DestroyFramebuffer(const FramebufferHandle &framebuffer) {
    glDeleteFramebuffers(1, &framebuffer.FBO);
    glDeleteTextures(1, &framebuffer.texture);
    glDeleteRenderbuffers(1, &framebuffer.depth);
}
