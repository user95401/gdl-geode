#include "stringPatch.hpp"

#include <Zydis/Zydis.h>
#include <string.h>
#include <chrono>
#include <PageManager.hpp>
#include <utils.hpp>

using namespace geode::prelude;

namespace gdl {
#if defined(GEODE_IS_WINDOWS64)
    bool patchCString(uintptr_t srcAddr, const char* str) {
        uint8_t* arr = (uint8_t*)srcAddr;
        if(arr[1] != 0x8D) {
            log::error("0x{:X}: instruction isn't lea!", srcAddr);
            return false;
        }

        uint8_t reg = arr[2] >> 3;

        std::array<uint8_t, 11> patch = {
            static_cast<uint8_t>(0x48 | (reg > 0x07)),      // prefix 64 bit operation
            static_cast<uint8_t>(0xB8 | (reg & 0xF7)),      // mov <reg>,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // <64-bit value>
            0xC3                                            // ret
        };

        *(uintptr_t*)(patch.data() + 2) = (uintptr_t)str;

        // because our instruction takes 10 bytes and the original one takes 7 bytes, we need to place a `call` in place of the original instruction that points to a trampoline
        // that we allocate which, in its turn, will contain the `mov` and `ret` instructions

        auto tramp = PageManager::get().getMemoryForSize(11); // + 1 because of `ret`
        std::copy(patch.begin(), patch.end(), tramp);

        std::array<uint8_t, 7> callBytes = {0xE8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90}; // call <tramp> and 2 NOPs
        auto relAddr = (uintptr_t)tramp - ((uintptr_t)srcAddr + 5);       // call is 5 bytes long
        *(int32_t*)(callBytes.data() + 1) = (int32_t)relAddr;

        if (Mod::get()->patch((void*)srcAddr, ByteVector(callBytes.begin(), callBytes.end())).isErr()) {
            log::error("0x{:X}: failed to patch!", srcAddr);
            return false;
        }

        return true;
    }

    bool patchStdString(uintptr_t srcAddr, std::string const& str) {
        // clang-format off
        // 1. patch the alloc_data function
        //   1. patch `lea rcx/ecx, [...]` OR `mov ecx/rcx ...` to the correct string size (with \0).
        //      i think that `mov ecx, <size>` is ok for all cases, because its 5 bytes (the smallest of all and can fit any 4byte int). FILL WITH NOPs!
        //      UPD: nvm. `lea ecx, [rdi+50h]` is 3 bytes. max for such instruction to fit in 3 bytes is 0x7f aka 127. there can also be 4 byte ones
        //      Well i guess we are just limited to that size OR we could place a `call` that would (a) `mov ecx, <size>` (b) `call alloc_data` (c) `ret` (and fill with nops). idk yet
        //   2. the next `mov ..., <NOT rax>` instruction is size, override it to the string len without \0. CAN BE A REGISTER INSTEAD OF VALUE!!! (in this case its smaller) (place call?)
        //   3. the next `mov ..., <NOT rax>` instruction is capacity, override it to the string len (with \0 i guess?). CAN BE A REGISTER INSTEAD OF VALUE!!! (in this case its smaller) (place call?)
        // 2. patch the string assignment
        //   1. probably just memcpy the (OWNED!) string data (with \0!) to `rax` and then `ret`
        //        lea     r8, [rbx+1]     ; Size
        //        mov     rdx, r12        ; Src
        //        mov     rcx, rax        ; void *
        //        call    memcpy
        //      (at 0x28643E)
        //   2. place bytes for ^ into a free page
        //   3. place `call` in place of original instructions (fill with nops all instructions that are `movups`, `movaps`, `movzx` (dont matter the operand bc it takes `eax`), `mov`
        //      that take `[<any reg> + ...]` as the first operand). there can be any register because it could do `mov rcx, rax` and then `mov [rcx+...], ...`
        // 3. shitty cases: 0x43A9A5, 0x43AA23 (fucked up order); 0x43A9A5, 0x43AA14 (3 byte lea ecx), 0x269D21, 3CB537 (correctly identified), 0x3CB297 (wtf)
        // 4. heck you compiler optimizations!!!
        // clang-format on

        // =========================================

        uint8_t* arr = (uint8_t*)srcAddr;
        uint8_t availableSize = 0;
        uint8_t reg = 0;
        

        // check for 64-bit operation
        if(*arr == 0x48 || *arr == 0x49) {
            arr++;
            availableSize++;
        }

        if(*arr == 0x8D) {                          // lea 
            auto modrm = gdlutils::decodeModRM(*arr);
            availableSize += modrm.size;
            reg = modrm.reg;
        } else if(*arr == 0x89 || *arr == 0x8B) {   // mov <reg>, <reg> || mov <reg>, [mem]
            auto modrm = gdlutils::decodeModRM(*arr);
            availableSize += modrm.size;
            reg = modrm.reg;
        } else if(*arr >= 0xB8 && *arr <= 0xBF) {   // mov <reg>, <imm>
            availableSize += 5;
            reg = (*arr - 0xB8) & 0xF7;
        } else {
            log::error("0x{:X}: Unknown operand {}!", srcAddr, arr[0]);
            return false;
        }

        bool strTooLong = 
            (availableSize <= 3 && str.size() > 0x7F) ||
            (availableSize == 4 && str.size() > 0x7FFF);

        if(strTooLong) {
            log::error("0x{:X}: String too long!", srcAddr, str.size());
            return false;
        }

        std::vector<uint8_t> patch(availableSize, 0x90);

        if(availableSize <= 3) {
            patch[0] = 0xB0 | (reg & 0xF7);
            patch[1] = static_cast<int8_t>(str.size());
        }

        if(availableSize == 4) {
            int16_t size = static_cast<int16_t>(str.size());
            patch[0] = 0x66;
            patch[1] = 0xB0 | (reg & 0xF7);
            std::copy(&patch[3], (uint8_t*)(&size),  (uint8_t*)(&size) + sizeof(size));
        }

        if(availableSize >= 5) {
            int32_t size = static_cast<int32_t>(str.size());
            patch[0] = 0xB8 | (reg & 0xF7);
            std::copy(&patch[1], (uint8_t*)(&size),  (uint8_t*)(&size) + sizeof(size));
        }

        if (Mod::get()->patch((void*)srcAddr, ByteVector(patch.begin(), patch.end())).isErr()) {
            log::error("0x{:X}: failed to patch std::string alloc size!", srcAddr);
            return false;
        }

        // To be continued...

        return true;
    }

#elif defined(GEODE_IS_ANDROID32)
    bool patchString(const uintptr_t dcd, const uintptr_t add, const char* str) {
        bool ret = true;

        ret &= Mod::get()->patch((void*)dcd, ByteVector {(uint8_t*)&str, (uint8_t*)&str + 4}).isOk();
        ret &= Mod::get()->patch((void*)add, ByteVector {0x00, 0xBF}).isOk();

        return ret;
    }
#endif
} // namespace gdl