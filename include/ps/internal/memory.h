#ifdef MXNET_USE_RDMA
#ifndef PS_MEMORY_H_
#define PS_MEMORY_H_

#include <memory>
#include "ps/internal/memory_allocator.h"

const static uint64_t kDefaultSize = 1ll * (1024 * 1024 * 1024);

class MR {
 public:
    MR() { }
    explicit MR(std::shared_ptr<brown::MemoryAllocator> ma, uintptr_t start) {
        ma_ = ma;
        start_ = start;
    }
    ~MR() { }
    void* Allocate(size_t size) {
        if (size < 256) size = 256;
        int64_t offset = ma_->Allocate(size);
        if (offset == brown::k_invalid_offset) {
            /* TODO(cjr) FIXME(cjr) Reallocate new region and set start_ point to it */
            assert(0);
        }
        return (void *)(start_ + offset);
    }
    void Deallocate(void *data) {
        //CHECK_EQ(start_ + offset_ == (uintptr_t)data) << "something unexpected happened";
        ma_->Deallocate(addr2offset(data));
    }
    int64_t addr2offset(void *addr) {
        CHECK(in_range(addr)) << "address not in memory region";
        return (int64_t)((uintptr_t)addr - start_);
    }
    bool in_range(void *addr) {
        return start_ <= (uintptr_t)addr && (uintptr_t)addr < start_ + kDefaultSize;
    }
    std::shared_ptr<brown::MemoryAllocator> ma_;
    uintptr_t start_;
};

class Memory {

public:
    static Memory* GetMemory() {
        static Memory memory_;
        return &memory_;
    }

    Memory() {
        ptr = malloc(kDefaultSize);
        allocator = brown::MemoryAllocator::Create(1, kDefaultSize);
        mr = new MR(allocator, (uintptr_t)ptr);
    }

    ~Memory() {
        free(ptr);
        allocator = nullptr;
        delete mr;
    }

    void *ptr;
    std::shared_ptr<brown::MemoryAllocator> allocator;
    MR *mr;
};

#endif
#endif
