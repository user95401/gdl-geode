#pragma once
#include <Geode/Geode.hpp>

#ifdef GEODE_IS_WINDOWS64

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

    void freeAll();

    // if the last page can fit those bytes, return it. otherwise, alloc a new page
    Page& getPageForSize(size_t size);
    uint8_t* getMemoryForSize(size_t size);

  private:
    Page& allocNewPage();

    std::vector<Page> m_pages;
};

#endif