#pragma once
#include <Geode/Geode.hpp>

#ifdef GEODE_IS_WINDOWS
    #ifdef GDLAPI_EXPORTING
        #define GDLAPI_DLL __declspec(dllexport)
    #else
        #define GDLAPI_DLL __declspec(dllimport)
    #endif
#else
    #define GDLAPI_DLL __attribute__((visibility("default")))
#endif

namespace gdl {
#if defined(GEODE_IS_WINDOWS64)

    /// @brief Patch a normal C string
    /// @param absAddr The ABSOLUTE address of the `lea` instruction
    /// @param str A string. Note that it will be duplicated so the orignal string can be freed
    /// @return Whether the patch was successful
    [[nodiscard]] bool patchCString(const uintptr_t srcAddr, const char* str);

    [[nodiscard]] bool patchStdString(const uintptr_t srcAddr, const char* str);

#endif
} // namespace gdl