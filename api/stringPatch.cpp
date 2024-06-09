#include "stringPatch.hpp"

#include <Zydis/Zydis.h>
#include <string.h>

using namespace geode::prelude;

class PageManager {
    struct Page {
        uint8_t* m_address;
        size_t m_totalSize;
        size_t m_offset; // aka used size
        size_t m_id;

        inline uint8_t* getOffsetAddress() { return m_address + m_offset; }
        inline bool canFit(size_t len) { return m_totalSize - m_offset >= len; }
        inline void reserve(size_t len) { m_offset = std::min(m_offset + len, m_totalSize); }
        void free() {
            if (m_address) {
                VirtualFree(m_address, m_totalSize, MEM_RELEASE);
                m_address = nullptr;
                m_totalSize = 0;
                m_offset = 0;
            }
        }
    };

  public:
    static PageManager& get() {
        static PageManager inst;
        return inst;
    }

    ~PageManager() { freeAll(); }

    void freeAll() {
        for (auto& page : m_pages)
            page.free();
    }

    // if the last page can fit those bytes, return it. otherwise, alloc a new page
    Page& getPageForSize(size_t size) {
        if (m_pages.size() > 0 && m_pages.back().canFit(size)) {
            return m_pages.back();
        } else {
            return allocNewPage();
        }
    }

  private:
    Page& allocNewPage() {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        const uint64_t PAGE_SIZE = sysInfo.dwPageSize;

        auto targetAddr = base::get() + 0x251000; // approximately the middle of the exe file

        void* newPageAddr = nullptr;

        uint64_t startAddr = (uint64_t(targetAddr) & ~(PAGE_SIZE - 1)); // round down to nearest page boundary
        uint64_t minAddr = std::min(startAddr - 0x7FFFFF00, (uint64_t)sysInfo.lpMinimumApplicationAddress);
        uint64_t maxAddr = std::max(startAddr + 0x7FFFFF00, (uint64_t)sysInfo.lpMaximumApplicationAddress);

        uint64_t startPage = (startAddr - (startAddr % PAGE_SIZE));

        uint64_t pageOffset = 1;
        while (1) {
            uint64_t byteOffset = pageOffset * PAGE_SIZE;
            uint64_t highAddr = startPage + byteOffset;
            uint64_t lowAddr = (startPage > byteOffset) ? startPage - byteOffset : 0;

            bool needsExit = highAddr > maxAddr && lowAddr < minAddr;

            if (highAddr < maxAddr) {
                void* outAddr = VirtualAlloc((void*)highAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                if (outAddr) {
                    newPageAddr = outAddr;
                    break;
                }
            }

            if (lowAddr > minAddr) {
                void* outAddr = VirtualAlloc((void*)lowAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                if (outAddr != nullptr) {
                    newPageAddr = outAddr;
                    break;
                }
            }

            pageOffset++;

            if (needsExit) {
                break;
            }
        }

        if (newPageAddr == nullptr) {
            log::error("page failed to alloc (what?)");
            throw std::runtime_error("gdl: page failed to alloc");
        }

        m_pages.push_back(Page {.m_address = (uint8_t*)newPageAddr, .m_totalSize = PAGE_SIZE, .m_offset = 0, .m_id = m_pages.size()});
        return m_pages.back();
    }

    std::vector<Page> m_pages;
};

namespace gdl {
    bool patchCString(const uintptr_t srcAddr, const char* str) {
        // 1. allocate memory near src addr (trampoline)
        // 2. write the instructions to it
        // 3. write the `call` to the allocated memory instruction to the src addr
        // 4. fill with `nop`

        // using zydis here because the instruction can have different bytes for different registers
        ZydisDisassembledInstruction instruction;
        if (ZYAN_SUCCESS(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, srcAddr, (void*)srcAddr, ZYDIS_MAX_INSTRUCTION_LENGTH, &instruction))) {
            if (instruction.info.opcode != 0x8d) {
                log::error("instruction is not lea!");
                return false;
            }
        } else {
            log::error("Failed to decomp the orig instruction!");
            return false;
        }

        static std::vector<const char*> strings;
        strings.push_back(_strdup(str)); // now we actually own the string

        // 1.

        auto& page = PageManager::get().getPageForSize(14); // mov + lea + ret = 14 bytes
        auto pageAddr = page.getOffsetAddress();

        uint8_t buffer[14];

        // 2.

        // mov r11, stringAddress (10 bytes)
        {
            uint8_t bytes[10] = {0x49, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            *(uintptr_t*)(bytes + 2) = (uintptr_t)strings.back();
            memcpy(buffer, bytes, sizeof(bytes));
        }

        // lea <original register>, [r11] (3 bytes)
        {
            // using zydis here because the instruction can have different bytes for different registers
            ZydisEncoderRequest req;
            memset(&req, 0, sizeof(req));

            req.mnemonic = ZYDIS_MNEMONIC_LEA;
            req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
            req.operand_count = 2;
            req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
            req.operands[0].reg.value = instruction.operands[0].reg.value; // use original register

            req.operands[1].type = ZYDIS_OPERAND_TYPE_MEMORY;
            req.operands[1].mem.base = ZYDIS_REGISTER_R11;
            req.operands[1].mem.displacement = 0;
            req.operands[1].mem.size = 8;

            ZyanU8 encoded_instruction[ZYDIS_MAX_INSTRUCTION_LENGTH];
            ZyanUSize encoded_length = sizeof(encoded_instruction);

            if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, encoded_instruction, &encoded_length))) {
                log::error("Failed to encode instruction!");
                return false;
            }

            memcpy(buffer + 10, encoded_instruction, encoded_length);
        }

        // ret (1 byte)
        buffer[13] = 0xc3;

        memcpy(pageAddr, buffer, sizeof(buffer));
        page.reserve(14);

        // 3.

        uint8_t newSrcBytes[] = {
            0xe8, 0x00, 0x00, 0x00, 0x00 // call ... (to go to our trampoline, using `call` instead of `jmp` so we can get back with `ret`)
        };

        const auto relAddr = (uint64_t)pageAddr - ((uint64_t)srcAddr + sizeof(newSrcBytes));

        *(int32_t*)(newSrcBytes + 1) = (int32_t)relAddr;

        if (Mod::get()->patch((void*)srcAddr, ByteVector(newSrcBytes, newSrcBytes + sizeof(newSrcBytes))).isErr())
            return false;

        // 4.

        if (Mod::get()->patch((uint8_t*)srcAddr + 5, {0x90, 0x90}).isErr()) // lea is 7 bytes, jmp is 5 bytes => 2 nops
            return false;
        
        return true;
    }

    bool patchStdString(const uintptr_t srcAddr, const char* str) {
        // uint8_t bytes[10] = {0x48,0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        //     *(uintptr_t*)(bytes + 2) = (uintptr_t)str;

        // Mod::get()->patch((void*)srcAddr, ByteVector(bytes, bytes + sizeof(bytes)));

        // это было наивное предположение
        
        return true;
    }
} // namespace gdl