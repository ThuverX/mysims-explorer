#pragma once

#include <map>
#include <string>

#include "lambert.hpp"
#include "fallback.hpp"
#include "terrainLightMapTinted.hpp"


struct Shader {
    std::string vertexSource;
    std::string fragmentSource;
};

enum class ShaderId: uint32_t {
    LAMBERT = 0x94773578,
    TERRAIN_LIGHT_MAP_TINTED = 0x224E7FEE,
    FALLBACK = 0
};

const std::map<ShaderId, Shader> SHADERS = {
    {ShaderId::LAMBERT, {LAMBERT_VERTEX_SHADER_SOURCE, LAMBERT_FRAGMENT_SHADER_SOURCE}},
    {ShaderId::TERRAIN_LIGHT_MAP_TINTED, {TERRAIN_LIGHT_MAP_TINTED_VERTEX_SHADER_SOURCE, TERRAIN_LIGHT_MAP_TINTED_FRAGMENT_SHADER_SOURCE}},
    {ShaderId::FALLBACK, {FALLBACK_VERTEX_SHADER_SOURCE, FALLBACK_FRAGMENT_SHADER_SOURCE}},
};
