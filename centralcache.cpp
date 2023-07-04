#include "include/centralcache.h"
#include "include/pagecache.h"

// 获取一个非空Span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size) {
    //先查找当前spanlist中是否有非空的span，有则返回之
    Span* it = list.Begin();
    while (it != list.End()) {//第二趟的申请第513块内存时进入了死循环，list的最后一块span 的prev和next指向了同一块地方
        if (it->_freeList != nullptr)
            return it;
        else
            it = it->_next;
    }
    //走到此处说明在当前list中并未找到非空的span对象，则需要向PageCache中申请
    //由于向PageCache申请span对象无需访问CentralCache临界资源，因此将桶锁解除
    list._mtx.unlock();

    //对PageCache整体加锁，申请span对象
    PageCache::GetInstance()->_pageMtx.lock();
    Span* span = PageCache::GetInstance()->NewPage(SizeClass::NumMovePage(size));
    span->_isUse = true;
    span->_objSize = size;//记录下span中对象的大小
    PageCache::GetInstance()->_pageMtx.unlock();

    //将申请好的span对象划分
    char* start = (char*)(span->_pageId << PAGE_SHIFT);
    size_t bytes = span->_n << PAGE_SHIFT;
    char* end = start + bytes;

    //1.先将一小块内存插入到span的list中
    span->_freeList = start;
    start += size;
    //2.将剩余内存尾插到span中，尾插保证相对位置不变
    void* tail = span->_freeList;
    // size_t i = 1;//此时span中已经有一块内存
    while (start < end) {
        // i++;
        NextObj(tail) = start;
        start += size;
        tail = NextObj(tail);
    }

    //细节错误：尾插后要将tail的next置空,导致越界访问
    NextObj(tail) = nullptr;

    //3.此时划分完后，实际上span中的内存仍连续，只不过每一块内存中都标识了下一块的内存位置
    //申请好span对象并切分后恢复桶锁以保证线程安全
    list._mtx.lock();
    list.PushFront(span);
    return span;
}

//从CentralCache获取一定数量的内存对象给ThreadCache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size) {
    //计算ThreadCache所获取的内存对象在哪个哈希桶中
    size_t index = SizeClass::Index(size);
    //加锁，保护获取的线程安全(桶锁)
    _spanLists[index]._mtx.lock();

    //从spanList中获取一块span
    Span* span = GetOneSpan(_spanLists[index], size);
    assert(span);
    assert(span->_freeList);

    start = span->_freeList;
    end = start;

    size_t i = 0;
    size_t actualNum = 1;
    while (i < batchNum - 1 && NextObj(end) != nullptr) {
        end = NextObj(end);
        i++, actualNum++;
    }

    span->_useCount += actualNum;

    span->_freeList = NextObj(end);
    NextObj(end) = nullptr;

    _spanLists[index]._mtx.unlock();

    return actualNum;
}

CentralCache CentralCache::_sInst;

void CentralCache::ReleaseListToSpan(void* start, size_t size)
{
    size_t index = SizeClass::Index(size);//计算索引
    _spanLists[index]._mtx.lock();//桶锁保证释放时的线程安全

    while (start)
    {
        void* next = NextObj(start);

        //寻找start所映射的那块span
        Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
        //将start头插进span中
        NextObj(start) = span->_freeList;
        span->_freeList = start;
        //归还一块切好的内存后，usecount--
        span->_useCount--;
        //span的useCount减为0后，说明这一块span已经空闲，将其归还给PageCache
        if (span->_useCount == 0)
        {
            //此处出错，在归还span给PageCache时未将span从_spanLists中删除，导致了死循环
            _spanLists[index].Erase(span);
            span->_prev = nullptr;
            span->_next = nullptr;
            span->_freeList = nullptr;
            //此时将span归还给PageCache的过程中，线程不再访问CentralCache的临界资源
            //因此可以将桶锁解除，让其他线程访问并申请内存
            //但是归还过程中需要保证Page Cache的线程安全，故Page Cache需加锁
            _spanLists[index]._mtx.unlock();
            PageCache::GetInstance()->_pageMtx.lock();
            PageCache::GetInstance()->RealeaseSpanToPageCache(span);
            PageCache::GetInstance()->_pageMtx.unlock();
            //将span归还后要将桶锁恢复，防止出现线程安全问题
            _spanLists[index]._mtx.lock();
        }
        start = next;
    }
    _spanLists[index]._mtx.unlock();
}