#pragma once

#define MAKE_SINGLETON(ClassName) \
private: \
    ClassName() = default; \
    ClassName(const ClassName&) = delete; \
    ClassName(ClassName&&) = delete; \
    ClassName &operator=(const ClassName &) = delete; \
    ClassName &operator=(ClassName &&) = delete; \
public: \
    static ClassName &Get() { \
        static ClassName instance; \
        return instance; \
    }
