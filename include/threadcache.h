#pragma once

#include "common.h"
using namespace lh::mp;

namespace lh {
    namespace mp {

        class ThreadCache {
        public:
            // 申请和释放内存对象
            void* Allocate(size_t size);////从Thread Cache中申请size个Bytes的空间
            void DeAllocate(void* ptr, size_t size);

            void* FetchFromCentralCache(size_t index, size_t size); //向CentralCache中申请空间
            void ListTooLong(FreeList& list, size_t size); //释放对象导致链表过长时， 归还内存到CentralCache

        private:
            FreeList _freeLists[NFREELIST];
        };

        //TLS: thread local storage
        static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;
    }
}