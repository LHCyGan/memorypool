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
        static const size_t MAX_BYTE = 256 * 1024; //Thread Cache�д洢������ڴ��С--256Kb
        static const size_t NFREELIST = 208; //����������������
        static const size_t NPAGES = 129; //PageCache�е����ҳ����ӳ��Ͱ���������
        static const size_t PAGE_SHIFT = 13; //PAGE_ID�������һҳ�ڴ��С �� 8kΪ����ӳ��

        inline static void* SystemAlloc(size_t kPage) {
        #ifdef _WIN32
            void* ptr = VirtualAlloc(0, kPage << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        #elif __linux__
            // linux brk mmap��
            void* ptr = mmap(NULL, kPage << PAGE_SHIFT, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        #endif

            //���쳣
            if (ptr == nullptr)
                throw std::bad_alloc();

            return ptr;
        }

        inline static void SystemFree(void* ptr, size_t kPage) {
        #ifdef _WIN32
            VirtualFree(ptr, 0, MEM_RELEASE);
        #else
            // linux sbrk munmap��
            munmap(ptr, kPage << PAGE_SHIFT);
        #endif
        }

        /**
         * ȡ��һ������ڴ��е�ǰ4���ֽڣ�32λϵͳ������8���ֽڣ�64λϵͳ�����ڴ����洢һ��ָ��ָ����һ���ͷŻ��������ɶ����ڴ棬
         * ��ô������Ϊ�˲���ƽ̨ϵͳ���жϣ�����ʹ��һ��С���ɣ����������ڴ�ǿת��void**�����ͣ���ô�ٶ��������ָ�����ͽ�����
         * �Ϳ���ȡ����ǰ�����ǰ4���ֽڣ�32λϵͳ����8���ֽڣ�64λϵͳ����
         * @param obj
         * @return
         */
        static inline void*& NextObj(void* obj) {
            return *(void**)obj;
        }

        class SizeClass {
        public:
            // ������������10%���ҵ�����Ƭ�˷�
            // [1,128] 8byte����       freelist[0,16)
            // [128+1,1024] 16byte����   freelist[16,72)
            // [1024+1,8*1024] 128byte����   freelist[72,128)
            // [8*1024+1,64*1024] 1024byte����     freelist[128,184)
            // [64*1024+1,256*1024] 8*1024byte����   freelist[184,208)
            
            static inline size_t _RoundUp(size_t bytes, size_t alignNum) {
                return (bytes + (alignNum - 1) & ~(alignNum - 1));//��λ�������Ч��
            }

            // �����С����
            static inline size_t RoundUp(size_t size) {
                //����������ڴ��С��������������������������
                if (size <= 128) {
                    return _RoundUp(size, 8);//��8byte����
                }
                else if (size <= 1024) {
                    return _RoundUp(size, 16);//��16byte����
                }
                else if (size <= 8 * 1024) {
                    return _RoundUp(size, 128);//��128byte����
                }
                else if (size <= 64 * 1024) {
                    return _RoundUp(size, 1024);//��1024byte����
                }
                else if (size <= 256 * 1024) {
                    return _RoundUp(size, 8 * 1024); //��8*1024byte����
                }
                else { //����256KB    
                    return _RoundUp(size, 1 << PAGE_SHIFT);
                }

            }

            //�����ڶ�Ӧ��Ͱ�е��±�
            static inline size_t _Index(size_t bytes, size_t align_shift) {
                return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
            }

            //�����ڶ�Ӧ��Ͱ�е��±�
            static inline size_t Index(size_t size) {
                assert(size <= MAX_BYTE);
                static int group_array[4] = { 16, 56, 56, 56 }; //���ÿ�������ж��ٸ���
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

            // ������CentralCacheһ�λ�ȡ���ε�����
            static size_t NumMoveSize(size_t size) {
                assert(size > 0);//��ȡ���ڴ�����ܴ�С�������0
                size_t num = MAX_BYTE / size; //���������ڴ��������
                //�������μ����ȡ����������[1,32768]����Χ����
                //��˿��ƻ�ȡ�Ķ���������Χ������[2, 512],��Ϊ����
                //С�����ȡ�����ζ࣬������ȡ��������
                if (num < 2)
                    num = 2;
                if (num > 512)
                    num = 512;
                return num;
            }

            //����һ����PageCache�������ҳ
            //size: ���������8byte - 256kb
            static size_t NumMovePage(size_t size) {
                size_t num = NumMoveSize(size);//��ǰsize������������
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
                //���黹���ڴ�����ͷ�����������
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
                //�����������е��ڴ��ͷɾ��ȥ
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
            size_t _maxSize = 1;//����������
            size_t  _size = 0; //����������
        };

        //����������ڴ�Ŀ�Ƚṹ
        struct Span {
            PAGE_ID _pageId = 0;//����ڴ���ʼҳid
            size_t _n = 0; //ҳ������

            Span* _prev = nullptr; //˫������ṹ
            Span* _next = nullptr;

            size_t _objSize = 0;//span������С���ڴ��С
            
            bool isUse = false;// ��ʶ���span�Ƿ����ڱ�ʹ��

            size_t _useCount = 0; // ��ǰspan�Ѿ��������thread cache���ڴ�����
            void* _freeList = nullptr;
        };

        //����Span�Ĵ�ͷ˫������ṹ
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
            std::mutex _mtx; //��ÿ����ϣͰ����һ�����Ա�֤�̰߳�ȫ
        };
    }
}

#endif //MEMORYPOOL_COMMON_H
