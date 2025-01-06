#include <PageManager.hpp>

using namespace geode::prelude;

void PageManager::freeAll() {
    for (auto& page : m_pages) {
        page.free();
    }
}

PageManager::Page& PageManager::getPageForSize(size_t size) {
    if (m_pages.size() > 0 && m_pages.back().canFit(size)) {
        return m_pages.back();
    } else {
        return allocNewPage();
    }
}

uint8_t* PageManager::getMemoryForSize(size_t size) {
    auto& page = getPageForSize(size);
    auto ret = page.getOffsetAddress();
    page.reserve(size);
    return ret;
}

PageManager::Page& PageManager::allocNewPage() {
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