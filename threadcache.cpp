#define _CRT_SECURE_NO_WARNINGS 1

#include "include/threadcache.h"
#include "include/centralcache.h"

using namespace lh::mp;

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size) {
    //预计获取的批次数量
    //慢反馈调节算法
    //1、刚开始向CentralCache获取的批次数量较小，申请太多可能用不完
    //2、当size需求逐渐增大时，batchNum就不断增大，直到上限
    //3、size越小，一次向CentralCache申请的批次数量就较大
    //4、size越大，一次向CentralCache申请的批次数量就较小
    size_t batchNum = min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));
    if (_freeLists[index].MaxSize() == batchNum)
        _freeLists[index].MaxSize() += 1;
    void* start = nullptr;
    void* end = nullptr;

    //向CentralCache申请一定数量的内存对象
    size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
    assert(actualNum);

    if (actualNum == 1)
        assert(start == end);
    else
        _freeLists[index].PushRange(NextObj(start), end, actualNum - 1);

    return start;
}


void* ThreadCache::Allocate(size_t size) {
    assert(size <= MAX_BYTE);

    size_t alignSize = SizeClass::RoundUp(size);//计算要申请的内存空间大小
    size_t index = SizeClass::Index(size);//计算在自由链表中的索引

    if (!_freeLists[index].Empty()) {
        return _freeLists[index].Pop();//自由链表非空，从中取内存
    }
    else {
        return FetchFromCentralCache(index, alignSize);//否则向中心缓存申请
    }
}

void ThreadCache::DeAllocate(void* ptr, size_t size) {
    assert(ptr);
    assert(size <= MAX_BYTE);

    size_t index = SizeClass::Index(size);

    //找到对应的自由链表桶，将对象插入
    _freeLists[index].Push(ptr);

    //当链表长度不低于一次批量申请的最大数量时，就归还链表中的对象给CentralCache
    if (_freeLists[index].Size() >= _freeLists[index].MaxSize())
    {
        ListTooLong(_freeLists[index], size);
    }
}

void ThreadCache::ListTooLong(FreeList& list, size_t size) {
    void* start = nullptr;
    void* end = nullptr;

    list.PopRange(start, end, list.MaxSize());

    CentralCache::GetInstance()->ReleaseListToSpan(start, size);

}

