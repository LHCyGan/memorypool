//
// Created by Dell on 2023/6/29.
//
#pragma once

#ifndef MEMORYPOOL_COMMON_H
#define MEMORYPOOL_COMMON_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <time.h>
#include <assert.h>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <atomic>

#ifdef _WIN32
#include <Windows.h>
#elif __linux__
    // linux
#include <sys/mman.h>
#endif

#ifdef _WIN64
typedef unsigned long long PAGE_ID;
#elif _WIN32
typedef size_t PAGE_ID;
#elif __linux__
    // linux
typedef size_t PAGE_ID;
#endif

namespace lh {
    namespace mp {
        static const size_t MAX_BYTE = 256 * 1024; //Thread Cache中存储的最大内存大小--256Kb
        static const size_t NFREELIST = 208; //自由链表的最大数量
        static const size_t NPAGES = 129; //PageCache中的最大页数，映射桶的最大数量
        static const size_t PAGE_SHIFT = 13; //PAGE_ID与申请的一页内存大小 按 8k为倍数映射

        inline static void* SystemAlloc(size_t kPage) {
#ifdef _WIN32
            void* ptr = VirtualAlloc(0, kPage << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif __linux__
            // linux brk mmap等
            void* ptr = mmap(NULL, kPage << PAGE_SHIFT, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif

            //抛异常
            if (ptr == nullptr)
                throw std::bad_alloc();

            return ptr;
        }

        inline static void SystemFree(void* ptr, size_t kPage) {
#ifdef _WIN32
            VirtualFree(ptr, 0, MEM_RELEASE);
#else
            // linux sbrk munmap等
            munmap(ptr, kPage << PAGE_SHIFT);
#endif
        }

        /**
         * 取出一块对象内存中的前4个字节（32位系统）或者8个字节（64位系统）的内存来存储一个指针指向下一块释放回来的自由对象内存，
         * 那么在这里为了不作平台系统的判断，可以使用一个小技巧，即将对象内存强转成void**的类型，那么再对这个二级指针类型解引用
         * 就可以取出当前对象的前4个字节（32位系统）或8个字节（64位系统）。
         * @param obj
         * @return
         */
        static inline void*& NextObj(void* obj) {
            return *(void**)obj;
        }


    }
}

#endif //MEMORYPOOL_COMMON_H
