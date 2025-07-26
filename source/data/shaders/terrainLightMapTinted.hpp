inline auto TERRAIN_LIGHT_MAP_TINTED_VERTEX_SHADER_SOURCE = R"(#version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec2 aTexCoord;
    layout(location = 2) in vec2 aTexCoord2;

    out vec2 uv0;
    out vec2 uv1;

    uniform mat4 uMVP;

    void main() {
        gl_Position = uMVP * vec4(aPos, 1.0);
        uv0 = aTexCoord;
        uv1 = aTexCoord2;
    }
)";

inline auto TERRAIN_LIGHT_MAP_TINTED_FRAGMENT_SHADER_SOURCE = R"(#version 330 core
    in vec2 uv0;
    in vec2 uv1;
    out vec4 FragColor;

    uniform sampler2D diffuseMap;
    uniform sampler2D ambientMap;

    void main() {
        vec4 diffuseSurface = texture(diffuseMap, uv0);
        vec4 lightMapColor = texture(ambientMap, uv1) * 2;
        FragColor = vec4(diffuseSurface.rgb * lightMapColor.rgb, diffuseSurface.a);
    }
)";
