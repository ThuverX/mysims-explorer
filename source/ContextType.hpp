#pragma once

#include <cstdint>

enum class ContextType : uint8_t {
    NONE,
    MODEL,
    MATERIAL,
    MATERIALSET,
    XML,
};
