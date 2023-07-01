//
// Created by Dell on 2023/6/29.
//
#pragma once

#ifndef MEMORYPOOL_OBJECTPOOL_H
#define MEMORYPOOL_OBJECTPOOL_H

#include <cstddef>

#include "common.h"
using namespace lh::mp;

namespace lh {
    namespace mp {
        template <class T> //一次申请T大小的内存空间
        class ObjectPool {
        public:
            T* New() {
                T* obj;
                if (_freelist != nullptr) {
                    //头删
                    obj = (T*)_freelist;
                    _freelist = *(void**)_freelist;
                }
                else {
                    if (_remainedBytes < sizeof(T)) {
                        //当前内存池中没有足以分配的内存，需要申请
                        _remainedBytes = 8 * 1024; //申请定长（8Kb）的内存空间
                        _memory = (char*)SystemAlloc(_remainedBytes >> PAGE_SHIFT); //申请定长（8Kb）的内存空间
                    }

                    //保证一次分配的空间够存放下当前平台的指针
                    size_t unity = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
                    obj = (T*)_memory;
                    _memory += unity;
                    _remainedBytes -= unity;
                }
                //new(obj) T 是一种 C++ 中的定位 new（placement new）语法，它可以在指定的内存地址上为一个对象分配内存并调用构造函数。
                // 具体来说，new(obj) T 的含义是在内存地址 obj 上调用类型 T 的构造函数来创建一个 T 类型的对象，并返回该对象的指针。
                //定位new显式调用T类型构造函数
                new(obj) T;
                return obj;
            }

            void Delete(T* obj) {
                //显式调用obj对象的析构函数,清理空间
                obj->~T();
                //将obj内存块头插
                *(void**)obj = _freelist;
                _freelist = obj;
            }
        private:
            char* _memory = nullptr; //存储一次申请一大块的内存,char*类型便于分配内存
            void* _freelist = nullptr; //将释放的内存回收链接
            size_t _remainedBytes; //_memory中剩余的内存空间

        };
    }

}
#endif //MEMORYPOOL_OBJECTPOOL_H
