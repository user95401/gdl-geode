#pragma once
#include <Geode/Geode.hpp>
#include <initializer_list>

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

    // All addresses are absolute!
    [[nodiscard]] bool patchStdString(const char* str, uintptr_t allocSizeInsn, uintptr_t sizeInsn, uintptr_t capacityInsn, std::vector<uintptr_t> assignInsns);
    
    // same as patchStdString but all addresses are relative to gd base (it will be added to all addresses)
    [[nodiscard]] bool patchStdStringRel(const char* str, uintptr_t allocSizeInsn, uintptr_t sizeInsn, uintptr_t capacityInsn, std::vector<uintptr_t> assignInsns);

#endif
} // namespace gdl