#include "Renderer.hpp"

#include <iostream>

GLuint Renderer::CreateShader(const ShaderCreateInfo &info) {
    // Compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &info.vertexShader, nullptr);
    glCompileShader(vertexShader);

    // Check vertex shader compile status
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR: Vertex shader compilation failed\n" << infoLog << std::endl;
    }

    // Compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &info.fragmentShader, nullptr);
    glCompileShader(fragmentShader);

    // Check fragment shader compile status
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR: Fragment shader compilation failed\n" << infoLog << std::endl;
    }

    // Link shaders into a program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check linking status
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR: Shader program linking failed\n" << infoLog << std::endl;
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

void Renderer::DestroyTexture(TextureHandle &texture) {
    if (texture != 0) {
        glDeleteTextures(1, &texture);
    }
}

MeshHandle Renderer::CreateMesh(const MeshCreateInfo &info) {
    MeshHandle mesh;

    mesh.texture = 0;
    mesh.indexCount = info.indices.size();
    
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, info.vertices.size() * sizeof(float), info.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, info.indices.size() * sizeof(uint32_t), info.indices.data(), GL_STATIC_DRAW);

    // position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texcoord attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return mesh;
}

void Renderer::DestroyMesh(MeshHandle &mesh) {
    glDeleteBuffers(1, &mesh.EBO);
    glDeleteBuffers(1, &mesh.VBO);
    glDeleteVertexArrays(1, &mesh.VAO);
}
