#pragma once

#include "common.h"
#include "threadcache.h"
#include "pagecache.h"
#include "objectpool.h"

using namespace lh::mp;

namespace lh {
    namespace mp {
        void* ConcurrentAlloc(size_t size) {
            // 若申请的内存大于256KB，则直接跳过CentralCache直接向Page Cache申请内存
            if (size > MAX_BYTE) {
                //先计算要申请的内存所对齐的大小
                size_t alignSize = SizeClass::RoundUp(size);//这里alignSize按size的对齐数向上对齐
                size_t kPage = alignSize >> PAGE_SHIFT;
                //向PageCache去申请内存
                PageCache::GetInstance()->_pageMtx.lock();
                Span* span = PageCache::GetInstance()->NewPage(kPage);
                span->_isUse = true;
                span->_objSize = size;
                PageCache::GetInstance()->_pageMtx.unlock();
                void* ptr = (void*)(span->_pageId << PAGE_SHIFT);

                return ptr;
            }
            else {
                //每个线程通过TLS各自创建自己私有的ThreadCache对象
                if (pTLSThreadCache == nullptr) {
                    //静态成员，保证全局只有一个对象
                    static ObjectPool<ThreadCache> tcPool;
                    //pTLSThreadCache = new ThreadCache;
                    pTLSThreadCache = tcPool.New();
                }
                return pTLSThreadCache->Allocate(size);
            }
        }

        void ConcurrentFree(void* ptr) {
            Span* span = PageCache::GetInstance()->MapObjectToSpan(ptr);
            size_t size = span->_objSize;
            //若释放的内存大于256KB，归还给Page Cache
            if (size > MAX_BYTE) {
                //找到ptr对应的那块span
                PageCache::GetInstance()->_pageMtx.lock();
                PageCache::GetInstance()->RealeaseSpanToPageCache(span);
                PageCache::GetInstance()->_pageMtx.unlock();
            }
            else {
                assert(pTLSThreadCache);
                pTLSThreadCache->DeAllocate(ptr, size);
            }
        }

    }

}