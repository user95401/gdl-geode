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
    /// @param str A string. Note that it must have static lifetime!
    /// @return Whether the patch was successful
    [[nodiscard]] GDLAPI_DLL bool patchCString(uintptr_t srcAddr, const char* str);

    /// @brief patch inline std::string
    /// @param srcAddr - absolute address of lea/mov instruction
    [[nodiscard]] GDLAPI_DLL bool patchStdString(uintptr_t absAddr, std::string const& str);
    
#elif defined(GEODE_IS_ANDROID32)
    [[nodiscard]] GDLAPI_DLL bool patchString(const uintptr_t srcAddr, const char* str);
#endif
} // namespace gdl