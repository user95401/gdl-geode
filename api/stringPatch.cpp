#include "stringPatch.hpp"

#include <Zydis/Zydis.h>
#include <string.h>

using namespace geode::prelude;

#if defined(GEODE_IS_WINDOWS64)

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

    uint8_t* getMemoryForSize(size_t size) {
        auto& page = getPageForSize(size);
        auto ret = page.getOffsetAddress();
        page.reserve(size);
        return ret;
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

#endif

namespace gdl {
#if defined(GEODE_IS_WINDOWS64)
    bool patchCString(uintptr_t srcAddr, const char* str) {
        ZydisDisassembledInstruction instruction;
        if (ZYAN_SUCCESS(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, srcAddr, (void*)srcAddr, ZYDIS_MAX_INSTRUCTION_LENGTH, &instruction))) {
            if (instruction.info.opcode != 0x8d) {
                log::error("instruction is not lea!");
                return false;
            }

            if (instruction.info.length != 7) {
                log::error("wtf instruction isnt 7 bytes long");
                return false;
            }
        } else {
            log::error("Failed to decomp the orig instruction!");
            return false;
        }

        static std::vector<const char*> strings;
        strings.push_back(_strdup(str)); // now we actually own the string

        // mov <original reg>, <string address>
        ZydisEncoderRequest req;
        memset(&req, 0, sizeof(req));

        req.mnemonic = ZYDIS_MNEMONIC_MOV;
        req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
        req.operand_count = 2;

        req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
        req.operands[0].reg.value = instruction.operands[0].reg.value;

        req.operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
        req.operands[1].imm.u = (uintptr_t)strings.back();

        ZyanU8 encoded_instruction[ZYDIS_MAX_INSTRUCTION_LENGTH];
        ZyanUSize encoded_length = sizeof(encoded_instruction);

        if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, encoded_instruction, &encoded_length))) {
            log::error("Failed to encode instruction!");
            return false;
        }

        // because our instruction takes 10 bytes and the original one takes 7 bytes, we need to place a `call` in place of the original instruction that points to a trampoline
        // that we allocate which, in its turn, will contain the `mov` and `ret` instructions

        auto tramp = PageManager::get().getMemoryForSize(encoded_length + 1); // + 1 because of `ret`

        memcpy(tramp, encoded_instruction, encoded_length);
        tramp[encoded_length] = 0xC3; // ret

        // now, the call instruction

        uint8_t callBytes[] = {0xE8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90}; // call <tramp> and 2 NOPs
        auto relAddr = (uintptr_t)tramp - ((uintptr_t)srcAddr + 5);       // call is 5 bytes long
        *(int32_t*)(callBytes + 1) = (int32_t)relAddr;

        if (Mod::get()->patch((void*)srcAddr, ByteVector(callBytes, callBytes + sizeof(callBytes))).isErr()) {
            log::error("failed to patch!");
            return false;
        }

        return true;
    }

    bool patchStdStringRel(const char* str, uintptr_t allocSizeInsn, uintptr_t sizeInsn, uintptr_t capacityInsn, std::vector<uintptr_t> assignInsns) {
        for (auto& el : assignInsns) {
            el += base::get();
        }
        return patchStdString(str, base::get() + allocSizeInsn, base::get() + sizeInsn, base::get() + capacityInsn, assignInsns);
    }

    bool patchStdString(const char* str, uintptr_t allocSizeInsn, uintptr_t sizeInsn, uintptr_t capacityInsn, std::vector<uintptr_t> assignInsns) {
        // clang-format off
        // 1. patch the alloc_data function
        //   1. patch `lea rcx/ecx, [...]` OR `mov ecx/rcx ...` to the correct string size (with \0).
        //      i think that `mov ecx, <size>` is ok for all cases, because its 5 bytes (the smallest of all and can fit any 4byte int). FILL WITH NOPs!
        //      UPD: nvm. `lea ecx, [rdi+50h]` is 3 bytes. max for such instruction to fit in 3 bytes is 0x7f aka 127.
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
        // 3. shitty cases: 0x43A9A5 (fucked up order); 0x43A9A5, 0x43AA14 (3 byte lea ecx)
        // 4. heck you compiler optimizations!!!
        // clang-format on

        // =========================================

        std::vector<std::shared_ptr<Patch>> patches;

        // 0
        ZydisDisassembledInstruction disasmInsn;
        auto stringLen = strlen(str);
        auto stringLenFull = stringLen + 1; // with \0
        auto capacity = stringLenFull;
        auto allocatingSize = capacity + 1;

        log::debug("string len {}, full {}, capacity {}, allocatingSize {}", stringLen, stringLenFull, capacity, allocatingSize);

        // 1.1
        {
            if (ZYAN_FAILED(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, allocSizeInsn, (void*)allocSizeInsn, ZYDIS_MAX_INSTRUCTION_LENGTH, &disasmInsn))) {
                log::error("failed to disasm instruction at allocSizeInsn (0x{:X})", allocSizeInsn);
                return false;
            }

            log::debug("{}", disasmInsn.text);
            log::debug("{} ops, {} {}", disasmInsn.info.operand_count, (int)disasmInsn.operands[0].type, (int)disasmInsn.operands[1].type);

            if (disasmInsn.info.operand_count != 2) {
                log::error("not 2 operands");
                return false;
            }

            if (disasmInsn.info.length < 5) { // mov ecx, <size> wouldn't fit
                if (allocatingSize > 0x7f) {
                    // uh oh, we don't have enough space
                    // we need to allocate some bytes and `call` them

                    log::error("todo");
                    return false;
                } else {
                    ZydisEncoderRequest req;
                    memset(&req, 0, sizeof(req));

                    req.mnemonic = ZYDIS_MNEMONIC_LEA;
                    req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
                    req.operand_count = 2;

                    req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
                    req.operands[0].reg.value = disasmInsn.operands[0].reg.value; // use original register

                    req.operands[1].type = ZYDIS_OPERAND_TYPE_MEMORY;
                    req.operands[1].mem.base = disasmInsn.operands[1].mem.base;
                    req.operands[1].mem.displacement = allocatingSize;
                    req.operands[1].mem.size = 8;

                    ZyanU8 encoded_instruction[ZYDIS_MAX_INSTRUCTION_LENGTH];
                    ZyanUSize encoded_length = sizeof(encoded_instruction);

                    if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, encoded_instruction, &encoded_length))) {
                        log::error("Failed to encode instruction [1]!");
                        return false;
                    }

                    std::string zvzv = "";
                    for (auto i = 0; i < encoded_length; i++) {
                        zvzv += std::format("{:02X} ", encoded_instruction[i]);
                    }
                    log::debug("{}", zvzv);

                    patches.push_back(Patch::create((void*)allocSizeInsn, ByteVector(encoded_instruction, encoded_instruction + encoded_length)));
                }
            } else {
                log::error("todo");
                return false;
            }
        }

        auto helpme = [&disasmInsn, &patches](uintptr_t insnAddr, size_t immValue) {
            if (ZYAN_FAILED(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, insnAddr, (void*)insnAddr, ZYDIS_MAX_INSTRUCTION_LENGTH, &disasmInsn))) {
                log::error("failed to disasm instruction at 0x{:X}", insnAddr);
                return false;
            }

            log::debug("{}", disasmInsn.text);
            log::debug("{} ops, {} {}", disasmInsn.info.operand_count, (int)disasmInsn.operands[0].type, (int)disasmInsn.operands[1].type);

            if (disasmInsn.info.operand_count != 2) {
                log::error("not 2 operands");
                return false;
            }

            if (disasmInsn.operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY) {
                log::error("not mem op");
                return false;
            }

            ZydisEncoderRequest req;
            memset(&req, 0, sizeof(req));

            req.mnemonic = disasmInsn.info.mnemonic;
            req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
            req.operand_count = 2;

            req.operands[0].type = ZYDIS_OPERAND_TYPE_MEMORY;
            req.operands[0].mem.base = disasmInsn.operands[0].mem.base;
            req.operands[0].mem.displacement = disasmInsn.operands[0].mem.disp.value;
            req.operands[0].mem.size = 8;

            req.operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
            req.operands[1].imm.u = immValue;

            ZyanU8 encoded_instruction[ZYDIS_MAX_INSTRUCTION_LENGTH];
            ZyanUSize encoded_length = sizeof(encoded_instruction);

            if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, encoded_instruction, &encoded_length))) {
                log::error("Failed to encode instruction [2]!");
                return false;
            }

            std::string zvzv = "";
            for (auto i = 0; i < encoded_length; i++) {
                zvzv += std::format("{:02X} ", encoded_instruction[i]);
            }
            log::debug("{}", zvzv);

            if (encoded_length < disasmInsn.info.length) {
                log::error("we dont fit =(");
                return false;
            } else {
                patches.push_back(Patch::create((void*)insnAddr, ByteVector(encoded_instruction, encoded_instruction + encoded_length)));
            }

            return true;
        };

        // 1.2
        if (!helpme(sizeInsn, stringLen))
            return false;

        // 1.3
        if (!helpme(capacityInsn, capacity))
            return false;

        {
            if (assignInsns.size() == 0) {
                log::error("you didnt specify any assign instructions");
                return false;
            }

            // 2.3
            bool hasCallPatch = false;
            uintptr_t callPatchAddr = 0;

            for (auto i = 0; i < assignInsns.size(); i++) {
                auto addr = assignInsns[i];

                if (ZYAN_FAILED(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, addr, (void*)addr, ZYDIS_MAX_INSTRUCTION_LENGTH, &disasmInsn))) {
                    log::error("failed to disasm instruction for step 2.3 at 0x{:X}", addr);
                    return false;
                }

                log::debug("{}", disasmInsn.text);
                log::debug("{} ops, {} {}", disasmInsn.info.operand_count, (int)disasmInsn.operands[0].type, (int)disasmInsn.operands[1].type);

                if (disasmInsn.info.operand_count != 2) {
                    log::error("not 2 operands");
                    return false;
                }

                auto instructionLength = disasmInsn.info.length;

                if (!hasCallPatch) {              // add the call patch
                    if (instructionLength >= 5) { // 5 bytes to fit the `call` instruction
                        callPatchAddr = addr;
                        hasCallPatch = true;

                        if (instructionLength > 5) { // fill the rest of it with nops
                            patches.push_back(Patch::create((void*)(addr + 5), ByteVector(instructionLength - 5, 0x90)));
                        }

                        continue;
                    }
                }

                patches.push_back(Patch::create((void*)addr, ByteVector(instructionLength, 0x90)));
            }

            if (!hasCallPatch) {
                log::error("all instructions cant fit call :(");
                return false;
            }

            // 2.1
            // clang-format off
            uint8_t bytes[] = {
                0x49, 0xC7, 0xC0, 0x00, 0x00, 0x00, 0x00, // mov r8, string len
                0x48, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rdx, string addr
                0x48, 0x89, 0xC1, // mov rcx, rax
                0xE8, 0x00, 0x00, 0x00, 0x00, // call memcpy
                0xC3 // ret
            };
            // clang-format on
            *(uint32_t*)(bytes + 3) = static_cast<uint32_t>(stringLenFull);
            *(uintptr_t*)(bytes + 9) = (uintptr_t)str;

            auto pageAddr = PageManager::get().getMemoryForSize(sizeof(bytes));

            const auto memcpyAddress = base::get() + 0x4A49F0;
            const auto relAddrMemcpy = (uint64_t)memcpyAddress - ((uint64_t)pageAddr + 20 + 5);
            log::debug("0x{:X}", relAddrMemcpy);
            *(int32_t*)(bytes + 21) = (int32_t)relAddrMemcpy;
            memcpy(pageAddr, bytes, sizeof(bytes));

            std::string zzzz;
            for (auto i = 0; i < sizeof(bytes); i++) {
                zzzz += std::format("{:02X} ", bytes[i]);
            }
            log::debug("{}", zzzz);

            uint8_t callBytes[] = {0xe8, 0x00, 0x00, 0x00, 0x00};
            const auto relAddr = (uint64_t)pageAddr - ((uint64_t)callPatchAddr + sizeof(callBytes));
            *(int32_t*)(callBytes + 1) = (int32_t)relAddr;
            patches.push_back(Patch::create((void*)callPatchAddr, ByteVector(callBytes, callBytes + sizeof(callBytes))));

            for (auto p : patches) {
                const auto& b = p->getBytes();

                std::string zvzv = "";
                for (auto i = 0; i < b.size(); i++) {
                    zvzv += std::format("{:02X} ", b[i]);
                }

                auto patchRes = Mod::get()->claimPatch(p);
                log::debug("Patch at 0x{:X}: {} {}", p->getAddress(), zvzv, patchRes.isOk());
                if (!patchRes)
                    log::error("Patch failed!!!");
            }
        }

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