inline auto LAMBERT_VERTEX_SHADER_SOURCE = R"(#version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec2 aTexCoord;

    out vec2 TexCoord;

    uniform mat4 uMVP;

    void main() {
        gl_Position = uMVP * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";

inline auto LAMBERT_FRAGMENT_SHADER_SOURCE = R"(#version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;

    uniform sampler2D diffuseMap;

    void main() {
        FragColor = texture(diffuseMap, TexCoord);
    }
)";
