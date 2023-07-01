#include "centralcache.h"
#include "pagecache.h"

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
	size_t i = 1;//此时span中已经有一块内存
	while (start < end) {
		i++;
		NextObj(tail) = start;
		start += size;
		tail = NextObj(tail);
	}

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