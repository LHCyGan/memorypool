//
// Created by Dell on 2023/7/5.
//

#ifndef MEMORYPOOL_PAGEMAP_H
#define MEMORYPOOL_PAGEMAP_H
#pragma once

#include "common.h"

// 单层基数树
namespace lh {
    namespace mp {
        //单层基数树
        template <size_t BITS>
        class TCMalloc_PageMap1
        {
        private:
            static const int LENGTH = 1 << BITS;
            void** _array;

        public:
            typedef uintptr_t Number;//存储指针的一个无符号整型类型
            explicit TCMalloc_PageMap1()//一次将数组所需空间开好
            {
                //计算数组开辟空间所需的大小
                size_t size = sizeof(void*) << BITS;
                size_t alignSize = SizeClass::_RoundUp(size, 1 << PAGE_SHIFT);
                //由于要开辟的空间是2M，已经很大了，故直接想堆申请
                _array = (void**)SystemAlloc(alignSize >> PAGE_SHIFT);
                memset(_array, 0, size);
            }
            void Set(Number key, void* v)//key的范围是[0, 2^BITS - 1],_pageId
            {
                _array[key] = v;
            }
            void* Get(Number key) const
            {
                if ((key >> BITS) > 0)//若key超出范围或还未被设置，则返回空
                {
                    return nullptr;
                }
                return _array[key];
            }
        };
    }
}
#endif //MEMORYPOOL_PAGEMAP_H
