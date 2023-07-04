#pragma once

#include "common.h"
using namespace lh::mp;

namespace lh {
    namespace mp {
        class CentralCache {
        public:
            static CentralCache* GetInstance() {
                return &_sInst;
            }

            // 获取一个非空Span;
            Span* GetOneSpan(SpanList& list, size_t byte_size);

            // 从CentralCache获取一定数量的内存对象给ThreadCache
            size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);

            // 将一定数量的对象释放到span跨度
            void ReleaseListToSpan(void* start, size_t size);

        private:
            SpanList _spanLists[NFREELIST]; //CentralCache的span分割方式与ThreadCache相同

        private:
            CentralCache() {}
            CentralCache(const CentralCache&) = delete;//禁用拷贝
            static CentralCache _sInst;//单例模式
        };

    }
}