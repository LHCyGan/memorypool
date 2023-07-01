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

        class SizeClass {
        public:
            // 整体控制在最多10%左右的内碎片浪费
            // [1,128] 8byte对齐       freelist[0,16)
            // [128+1,1024] 16byte对齐   freelist[16,72)
            // [1024+1,8*1024] 128byte对齐   freelist[72,128)
            // [8*1024+1,64*1024] 1024byte对齐     freelist[128,184)
            // [64*1024+1,256*1024] 8*1024byte对齐   freelist[184,208)
            
            static inline size_t _RoundUp(size_t bytes, size_t alignNum) {
                return (bytes + (alignNum - 1) & ~(alignNum - 1));//移位计算提高效率
            }

            // 对齐大小计算
            static inline size_t RoundUp(size_t size) {
                //给定申请的内存大小，计算其对齐数，对齐规则如上
                if (size <= 128) {
                    return _RoundUp(size, 8);//按8byte对齐
                }
                else if (size <= 1024) {
                    return _RoundUp(size, 16);//按16byte对齐
                }
                else if (size <= 8 * 1024) {
                    return _RoundUp(size, 128);//按128byte对齐
                }
                else if (size <= 64 * 1024) {
                    return _RoundUp(size, 1024);//按1024byte对齐
                }
                else if (size <= 256 * 1024) {
                    return _RoundUp(size, 8 * 1024); //按8*1024byte对齐
                }
                else { //大于256KB    
                    return _RoundUp(size, 1 << PAGE_SHIFT);
                }

            }

            //计算在对应链桶中的下标
            static inline size_t _Index(size_t bytes, size_t align_shift) {
                return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
            }

            //计算在对应链桶中的下标
            static inline size_t Index(size_t size) {
                assert(size <= MAX_BYTE);
                static int group_array[4] = { 16, 56, 56, 56 }; //打表，每个区间有多少个链
                if (size < 128) {
                    return _Index(size, 3);
                }
                else if (size <= 1024) {
                    return _Index(size - 128, 4) + group_array[0];
                }
                else if (size <= 8 * 1024) {
                    return _Index(size - 1024, 7) + group_array[1] + group_array[0];
                }
                else if (size <= 64 * 1024) {
                    return _Index(size - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
                }
                else if (size <= 256 * 1024) {
                    return _Index(size - 64 * 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
                }
                else {
                    assert(false);
                }
            }

            // 计算向CentralCache一次获取批次的数量
            static size_t NumMoveSize(size_t size) {
                assert(size > 0);//获取的内存对象总大小必须大于0
                size_t num = MAX_BYTE / size; //计算合理的内存对象数量
                //根据批次计算获取的数量会在[1,32768]，范围过大，
                //因此控制获取的对象数量范围在区间[2, 512],较为合理
                //小对象获取的批次多，大对象获取的批次少
                if (num < 2)
                    num = 2;
                if (num > 512)
                    num = 512;
                return num;
            }

            //计算一次向PageCache申请多少页
            //size: 单个对象从8byte - 256kb
            static size_t NumMovePage(size_t size) {
                size_t num = NumMoveSize(size);//当前size所需批次数量
                size_t nPage = num * size;
                nPage >>= PAGE_SHIFT;
                if (nPage == 0)
                    nPage = 1;
                return nPage;
            }
        };

        class FreeList {
        public:
            void Push(void* obj) {
                //将归还的内存块对象头插进自由链表
                NextObj(obj) = _freelist;
                _freelist = obj;
                ++_size;
            }

            void PushRange(void* start, void* end) {
                NextObj(end) = _freelist;
                _freelist = start;
                //_size += size;
            }

            void* Pop() {
                assert(_freelist);
                //将自由链表中的内存块头删出去
                void* obj = _freelist;
                _freelist = NextObj(obj);
                --_size;
                return obj;
            }

            void* PopRange(void*& start, void*& end, size_t n) {
                assert(n >= _size);
                start = _freelist;
                end = start;
                for (size_t i = 0; i < n - 1; i++) {
                    end = NextObj(end);
                }
                _freelist = NextObj(end);
                _size -= n;
                NextObj(end) = nullptr;
            }

            bool Empty() {
                return _freelist == nullptr;
            }

            size_t& MaxSize() {
                return _maxSize;
            }

            size_t& Size() {
                return _size;
            }

        private:
            void* _freelist = nullptr;
            size_t _maxSize = 1;//计算链表长度
            size_t  _size = 0; //计算链表长度
        };

        //管理多个大块内存的跨度结构
        struct Span {
            PAGE_ID _pageId = 0;//大块内存起始页id
            size_t _n = 0; //页的数量

            Span* _prev = nullptr; //双向链表结构
            Span* _next = nullptr;

            size_t _objSize = 0;//span中所切小块内存大小
            
            bool isUse = false;// 标识这块span是否正在被使用

            size_t _useCount = 0; // 当前span已经被分配给thread cache的内存数量
            void* _freeList = nullptr;
        };

        //链接Span的带头双线链表结构
        class SpanList {
        public:
            SpanList() {
                _head = new Span;
                _head->_next = _head;
                _head->_prev = _head;
            }

            Span* Begin() {
                return _head->_next;
            }

            Span* End() {
                return _head;
            }

            bool Empty() {
                return _head == _head->_next;
            }

            void PushFront(Span* span) {
                Insert(_head->_next, span);
            }

            Span* PopFront() {
                Span* front = _head->_next;
                Erase(front);
                return front;
            }

            void Insert(Span* pos, Span* newSpan) {
                assert(pos);
                assert(newSpan);
                Span* prev = pos->_prev;
                prev->_next = newSpan;
                pos->_prev = newSpan;
                newSpan->_prev = prev;
                newSpan->_next = pos;
            }

            void Erase(Span* pos) {
                assert(pos);
                assert(pos != _head);
                Span* prev = pos->_prev;
                Span* next = pos->_next;
                prev->_next = next;
                next->_prev = prev;
            }

        private:
            Span* _head;
        public:
            std::mutex _mtx; //对每个哈希桶都加一个锁以保证线程安全
        };
    }
}

#endif //MEMORYPOOL_COMMON_H
