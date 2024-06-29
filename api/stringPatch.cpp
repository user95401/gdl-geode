#include "stringPatch.hpp"

#include <Zydis/Zydis.h>
#include <string.h>
#include <chrono>

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

        // mov <original reg>, <string address>
        ZydisEncoderRequest req;
        memset(&req, 0, sizeof(req));

        req.mnemonic = ZYDIS_MNEMONIC_MOV;
        req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
        req.operand_count = 2;

        req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
        req.operands[0].reg.value = instruction.operands[0].reg.value;

        req.operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
        req.operands[1].imm.u = (uintptr_t)str;

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

    bool patchStdString2(const char* str, const std::vector<PatchBlock>& blocks, uintptr_t bufAssignInsn) {
        log::debug("===================================================");

        if (blocks.size() == 0 || blocks[0].len < 5) {
            log::error("invalid block data");
            return false;
        }

        // disasm the assign insn
        ZydisDisassembledInstruction instruction;
        if (ZYAN_FAILED(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, bufAssignInsn, (void*)bufAssignInsn, ZYDIS_MAX_INSTRUCTION_LENGTH, &instruction))) {
            log::error("failed to disassemble the instruction at 0x{:X}", bufAssignInsn);
            return false;
        }

        log::debug("{}", instruction.text);

        if (instruction.operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY) {
            log::error("not mem op");
            return false;
        }

        auto size = strlen(str);
        auto capacity = std::max(0x10ull, size + 1);

        // jmp:
        //   mov rcx, <cap>
        //   call sub_140039BE0
        //   <buf assign insn>
        //   mov [<buf mem op> + 16], <size>
        //   mov [<buf mem op> + 24], <cap>
        //   mov r8, <size+1>
        //   mov rdx, <str>
        //   mov rcx, rax
        //   call memcpy
        //   jmp <blocks[0].end> ; aka start + len

        // encode size insn
        ZydisEncoderRequest req;

        memset(&req, 0, sizeof(req));
        req.mnemonic = ZYDIS_MNEMONIC_MOV;
        req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
        req.operand_count = 2;

        req.operands[0].type = ZYDIS_OPERAND_TYPE_MEMORY;
        req.operands[0].mem.base = instruction.operands[0].mem.base;
        req.operands[0].mem.displacement = instruction.operands[0].mem.disp.value + 16;
        req.operands[0].mem.size = 8;

        req.operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
        req.operands[1].imm.u = size;

        ZyanU8 encoded_instruction1[ZYDIS_MAX_INSTRUCTION_LENGTH];
        ZyanUSize encoded_length1 = sizeof(encoded_instruction1);

        if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, encoded_instruction1, &encoded_length1))) {
            log::error("Failed to encode instruction [1]!");
            return false;
        }

        {
            std::string zvzv = "";
            for (auto i = 0; i < encoded_length1; i++) {
                zvzv += std::format("{:02X} ", encoded_instruction1[i]);
            }
            log::debug("{}", zvzv);
        }

        memset(&req, 0, sizeof(req));
        req.mnemonic = ZYDIS_MNEMONIC_MOV;
        req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
        req.operand_count = 2;

        req.operands[0].type = ZYDIS_OPERAND_TYPE_MEMORY;
        req.operands[0].mem.base = instruction.operands[0].mem.base;
        req.operands[0].mem.displacement = instruction.operands[0].mem.disp.value + 24;
        req.operands[0].mem.size = 8;

        req.operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
        req.operands[1].imm.u = capacity;

        ZyanU8 encoded_instruction2[ZYDIS_MAX_INSTRUCTION_LENGTH];
        ZyanUSize encoded_length2 = sizeof(encoded_instruction2);

        if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, encoded_instruction2, &encoded_length2))) {
            log::error("Failed to encode instruction [2]!");
            return false;
        }

        {
            std::string zvzv = "";
            for (auto i = 0; i < encoded_length2; i++) {
                zvzv += std::format("{:02X} ", encoded_instruction2[i]);
            }
            log::debug("{}", zvzv);
        }

        // clang-format off
        auto tramp = PageManager::get().getMemoryForSize(0x2a + instruction.info.length + encoded_length1 + encoded_length2);
        auto offset = 0;

        tramp[offset] = 0x48; tramp[offset+1] = 0xc7; tramp[offset+2] = 0xc1;
        *(uint32_t*)(tramp + offset + 3) = capacity;
        offset += 7;

        tramp[offset] = 0xe8;
        auto relAddr = (uint64_t)(base::get() + 0x39BE0) - ((uint64_t)tramp + offset + 5);
        *(int32_t*)(tramp + offset + 1) = (int32_t)relAddr;
        offset += 5;

        memcpy(tramp + offset, (void*)bufAssignInsn, instruction.info.length);
        offset += instruction.info.length;

        memcpy(tramp + offset, encoded_instruction1, encoded_length1);
        offset += encoded_length1;

        memcpy(tramp + offset, encoded_instruction2, encoded_length2);
        offset += encoded_length2;

        tramp[offset] = 0x49; tramp[offset+1] = 0xc7; tramp[offset+2] = 0xc0;
        *(uint32_t*)(tramp + offset + 3) = size + 1;
        offset += 7;

        tramp[offset] = 0x48; tramp[offset+1] = 0xba;
        *(uintptr_t*)(tramp + offset + 2) = (uintptr_t)str;
        offset += 10;

        tramp[offset] = 0x48; tramp[offset+1] = 0x89; tramp[offset+2] = 0xc1;
        offset += 3;

        tramp[offset] = 0xe8;
        relAddr = (uint64_t)(base::get() + 0x4A49F0) - ((uint64_t)tramp + offset + 5);
        *(int32_t*)(tramp + offset + 1) = (int32_t)relAddr;
        offset += 5;

        tramp[offset] = 0xe9;
        relAddr = (uint64_t)(blocks[0].start + blocks[0].len) - ((uint64_t)tramp + offset + 5);
        *(int32_t*)(tramp + offset + 1) = (int32_t)relAddr;
        
        // clang-format on

        {
            std::string zvzv = "";
            for (auto i = 0; i < offset + 1; i++) {
                zvzv += std::format("{:02X} ", tramp[i]);
            }
            log::debug("{}", zvzv);
        }

        uint8_t patch[] = {0xe9, 0x00, 0x00, 0x00, 0x00}; // jmp <tramp>
        relAddr = (uint64_t)tramp - ((uint64_t)blocks[0].start + 5);
        *(int32_t*)(patch + 1) = (int32_t)relAddr;

        if (auto p = Mod::get()->patch((void*)blocks[0].start, ByteVector(patch, patch + sizeof(patch))); p.isErr()) {
            log::error("patch error {}", p.error());
            return false;
        }

        for (auto i = 1; i < blocks.size(); i++) {
            auto& block = blocks[i];
            if (block.len < 5) {
                if (auto p = Mod::get()->patch((void*)block.start, ByteVector(block.len, 0x90)); p.isErr()) {
                    log::error("patch error {}", p.error());
                    return false;
                }
            } else {
                uint8_t patch2[] = {0xe9, 0x00, 0x00, 0x00, 0x00};
                *(int32_t*)(patch2 + 1) = block.len - 5;
                if (auto p = Mod::get()->patch((void*)block.start, ByteVector(patch2, patch2 + sizeof(patch2))); p.isErr()) {
                    log::error("patch error {}", p.error());
                    return false;
                }
            }
        }

        return true;
    }

    bool patchStdString(const char* str_, uintptr_t allocSizeInsn, uintptr_t sizeInsn, uintptr_t capacityInsn, std::vector<uintptr_t> assignInsns) {
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

        log::error("TODO PATCH ASSIGN AS LATE AS POSSIBLE"); // maybe in <16 only?
        return false;

        auto str = _strdup(str_);
        static std::vector<const char*> strings;
        strings.push_back(str);

        std::vector<std::shared_ptr<Patch>> patches;

        // 0
        ZydisDisassembledInstruction disasmInsn;
        auto stringLen = strlen(str);
        auto stringLenFull = stringLen + 1; // with \0
        auto capacity =
            std::max(stringLenFull,
                     0x10ull); // use bigger capacity to ensure that smaller strings (<= 15 bytes in length) work properly (if new string is < 16 chars and the orig is >= 16 chars)
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

            const auto srcLen = disasmInsn.info.length;

            if (srcLen < 5) {                 // mov ecx, <size> won't fit
                if (allocatingSize <= 0x7f) { // the original instruction will fit
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
                } else { // the original instruction wont fit
                    // because the instruction takes less than 5 bytes and call takes 5 bytes, we also need to copy the next instruction into the tramp.
                    // if the next insn is `call alloc_fn`, then change the address because it is rip-relative
                    ZydisDisassembledInstruction nextInsn;
                    const auto nextAddr = allocSizeInsn + srcLen;
                    if (ZYAN_FAILED(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, nextAddr, (void*)nextAddr, ZYDIS_MAX_INSTRUCTION_LENGTH, &nextInsn))) {
                        log::error("failed to disasm instruction at nextAddr (0x{:X})", nextAddr);
                        return false;
                    }
                    const auto nextLen = nextInsn.info.length;

                    const auto trampLen = 5 + nextLen + 1;
                    auto trampAddr = PageManager::get().getMemoryForSize(trampLen); // mov, next instruction, ret
                    trampAddr[0] = 0xB9;                                            // mov
                    *(uint32_t*)(trampAddr + 1) = (uint32_t)allocatingSize;
                    trampAddr[trampLen - 1] = 0xC3; // ret

                    memcpy(trampAddr + 5, (void*)nextAddr, nextLen);
                    if (nextInsn.info.mnemonic == ZYDIS_MNEMONIC_CALL) {
                        const auto resultingAddress = nextAddr + nextLen + nextInsn.operands[0].imm.value.u;

                        const auto relAddr = (uint64_t)resultingAddress - ((uint64_t)trampAddr + 5 + nextLen);
                        *(int32_t*)(trampAddr + 5 + 1) = (int32_t)relAddr;
                    }

                    auto srcTotalLen = srcLen + nextLen;
                    auto srcPatchBytes = new uint8_t[srcTotalLen];
                    memset(srcPatchBytes, 0x90, srcTotalLen);
                    srcPatchBytes[0] = 0xE8;

                    const auto relAddr = (uint64_t)trampAddr - ((uint64_t)allocSizeInsn + 5);
                    *(int32_t*)(srcPatchBytes + 1) = (int32_t)relAddr;

                    patches.push_back(Patch::create((void*)allocSizeInsn, ByteVector(srcPatchBytes, srcPatchBytes + srcTotalLen)));
                }
            } else {
                // mov ecx, <val> and fill rest with nops
                auto bytes = new uint8_t[srcLen];
                memset(bytes, 0x90, srcLen); // fill with nops
                bytes[0] = 0xB9;
                *(uint32_t*)(bytes + 1) = (uint32_t)allocatingSize;
                patches.push_back(Patch::create((void*)allocSizeInsn, ByteVector(bytes, bytes + srcLen)));
                delete[] bytes;
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

            req.mnemonic = ZYDIS_MNEMONIC_MOV;
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

            const auto srcLen = disasmInsn.info.length;

            if (srcLen < encoded_length) {
                if (srcLen >= 5) { // enough space for a call
                    auto tramp = PageManager::get().getMemoryForSize(encoded_length + 1);
                    memcpy(tramp, encoded_instruction, encoded_length);
                    tramp[encoded_length] = 0xc3; // ret

                    auto bytes = new uint8_t[srcLen];
                    memset(bytes, 0x90, srcLen);
                    bytes[0] = 0xe8;

                    const auto relAddr = (uint64_t)tramp - ((uint64_t)insnAddr + 5);
                    *(int32_t*)(bytes + 1) = (int32_t)relAddr;

                    patches.push_back(Patch::create((void*)insnAddr, ByteVector(bytes, bytes + srcLen)));

                    delete[] bytes;
                } else {
                    // because the instruction takes less than 5 bytes and call takes 5 bytes, we also need to copy the next instruction into the tramp.
                    // if the next insn is `call alloc_fn`, then change the address because it is rip-relative
                    ZydisDisassembledInstruction nextInsn;
                    const auto nextAddr = insnAddr + srcLen;
                    if (ZYAN_FAILED(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, nextAddr, (void*)nextAddr, ZYDIS_MAX_INSTRUCTION_LENGTH, &nextInsn))) {
                        log::error("failed to disasm instruction at nextAddr (0x{:X})", nextAddr);
                        return false;
                    }
                    const auto nextLen = nextInsn.info.length;

                    const auto trampLen = encoded_length + nextLen + 1;
                    auto trampAddr = PageManager::get().getMemoryForSize(trampLen); // mov, next instruction, ret
                    memcpy(trampAddr, encoded_instruction, encoded_length);
                    trampAddr[trampLen - 1] = 0xC3; // ret

                    memcpy(trampAddr + encoded_length, (void*)nextAddr, nextLen);
                    if (nextInsn.info.mnemonic == ZYDIS_MNEMONIC_CALL) {
                        const auto resultingAddress = nextAddr + nextLen + nextInsn.operands[0].imm.value.u;

                        const auto relAddr = (uint64_t)resultingAddress - ((uint64_t)trampAddr + encoded_length + nextLen);
                        *(int32_t*)(trampAddr + encoded_length + 1) = (int32_t)relAddr;
                    }

                    auto srcTotalLen = srcLen + nextLen;
                    auto srcPatchBytes = new uint8_t[srcTotalLen];
                    memset(srcPatchBytes, 0x90, srcTotalLen);
                    srcPatchBytes[0] = 0xE8;

                    const auto relAddr = (uint64_t)trampAddr - ((uint64_t)insnAddr + 5);
                    *(int32_t*)(srcPatchBytes + 1) = (int32_t)relAddr;

                    patches.push_back(Patch::create((void*)insnAddr, ByteVector(srcPatchBytes, srcPatchBytes + srcTotalLen)));
                }
            } else {
                patches.push_back(Patch::create((void*)insnAddr, ByteVector(encoded_instruction, encoded_instruction + encoded_length)));
                if (srcLen > encoded_length) {
                    log::debug("[2] more!");
                    patches.push_back(Patch::create((void*)(insnAddr + encoded_length), ByteVector(srcLen - encoded_length, 0x90)));
                }
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
                // wontfix
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