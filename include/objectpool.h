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
        template <class T> //һ������T��С���ڴ�ռ�
        class ObjectPool {
        public:
            T* New() {
                T* obj;
                if (_freelist != nullptr) {
                    //ͷɾ
                    obj = (T*)_freelist;
                    _freelist = *(void**)_freelist;
                }
                else {
                    if (_remainedBytes < sizeof(T)) {
                        //��ǰ�ڴ����û�����Է�����ڴ棬��Ҫ����
                        _remainedBytes = 8 * 1024; //���붨����8Kb�����ڴ�ռ�
                        _memory = (char*)SystemAlloc(_remainedBytes >> PAGE_SHIFT); //���붨����8Kb�����ڴ�ռ�
                    }

                    //��֤һ�η���Ŀռ乻����µ�ǰƽ̨��ָ��
                    size_t unity = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
                    obj = (T*)_memory;
                    _memory += unity;
                    _remainedBytes -= unity;
                }
                //new(obj) T ��һ�� C++ �еĶ�λ new��placement new���﷨����������ָ�����ڴ��ַ��Ϊһ����������ڴ沢���ù��캯����
                // ������˵��new(obj) T �ĺ��������ڴ��ַ obj �ϵ������� T �Ĺ��캯��������һ�� T ���͵Ķ��󣬲����ظö����ָ�롣
                //��λnew��ʽ����T���͹��캯��
                new(obj) T;
                return obj;
            }

            void Delete(T* obj) {
                //��ʽ����obj�������������,����ռ�
                obj->~T();
                //��obj�ڴ��ͷ��
                *(void**)obj = _freelist;
                _freelist = obj;
            }
        private:
            char* _memory = nullptr; //�洢һ������һ�����ڴ�,char*���ͱ��ڷ����ڴ�
            void* _freelist = nullptr; //���ͷŵ��ڴ��������
            size_t _remainedBytes; //_memory��ʣ����ڴ�ռ�

        };
    }

}
#endif //MEMORYPOOL_OBJECTPOOL_H
