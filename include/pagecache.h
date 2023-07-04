#pragma once

#include "common.h"
#include "objectpool.h"
using namespace lh::mp;

namespace lh {
    namespace mp {
        class PageCache {
        public:
            static PageCache* GetInstance() {
                return &_sInst;
            }

            //建立从对象到span*的映射
            Span* MapObjectToSpan(void* obj);

            //将CentralCache归还的span链接到PageCache对应的桶中，并且若其前后页有空闲，合并之
            void RealeaseSpanToPageCache(Span* span);

            // 获取一个k页的PageCache
            Span* NewPage(size_t k);
            //与CentralCache不同的是，若此处设置桶锁，会频繁的加锁解锁，故PageCache中整体设置一把锁
            std::mutex _pageMtx;

        private:
            PageCache() {}
            PageCache(const PageCache&) = delete;

            //定义定长的span内存池以脱离使用new
            ObjectPool<Span> _spanPool;

            std::unordered_map<PAGE_ID, Span*> _idSpanMap;

            static PageCache _sInst;
            SpanList _spanlists[NPAGES];
        };
    }
}