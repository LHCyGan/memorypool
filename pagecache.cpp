#include "include/pagecache.h"

PageCache PageCache::_sInst;

Span* PageCache::NewPage(size_t k) {
    assert(k);

    if (k > NPAGES - 1) {
        void* ptr = SystemAlloc(k);
        Span* span = _spanPool.New();

        span->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
        span->_n = k;

        // 建立映射，方便回收
        _idSpanMap[span->_pageId] = span;
        return span;
    }

    //若当前k页Page中有空闲的Span，返回
    if (!_spanlists[k].Empty()) {
        //在返回span之前，需要先建立映射
        Span* kSpan = _spanlists[k].PopFront();
        _idSpanMap[kSpan->_pageId] = kSpan;
        _idSpanMap[kSpan->_pageId + kSpan->_n - 1] = kSpan;
        //return _spanlists[k].PopFront();

        return kSpan;
    }

    //若当前k页Page中无空闲的Span，则向后寻找更大页的Page去切分出需要的Span来
    for (size_t i = k + 1; i < NPAGES; i++) {
        if (!_spanlists[i].Empty()) {
            //若i页Span非空，则将其切分
            //Span* kSpan = new Span;
            Span* kSpan = _spanPool.New();
            Span* nSpan = _spanlists[i].PopFront();

            //将nPage切分成k页Span及n - k页Span
            kSpan->_pageId = nSpan->_pageId;
            kSpan->_n = k;

            nSpan->_pageId += k;//细节错误：+=k写出-=k导致越界访问
            nSpan->_n -= k;
            //将切分后的nPage挂接到n-k页的Span下
            _spanlists[i - k].PushFront(nSpan);

            //nSpan的首尾id需要与Span建立映射，便于后续合并小page时的查询
            _idSpanMap[nSpan->_pageId] = nSpan;
            _idSpanMap[nSpan->_pageId + nSpan->_n - 1] = nSpan;

            //kSpan建立起id与Span的映射关系，便于Central Cache回收小块内存时查找对应的span
            for (size_t i = 0; i < kSpan->_n; i++) {
                //kSpan的每一页id都要与span建立映射，这样后续回收时才方便查找
                _idSpanMap[kSpan->_pageId + i] = kSpan;
            }

            //返回切好的kSpan
            return kSpan;
        }
    }

    //走到此处说明PageCache中没有可以使用的Span了
    //则需要向系统（堆）申请一大块内存(128页)以备用
    //Span* bigSpan = new Span;
    Span* bigSpan = _spanPool.New();

    void* ptr = SystemAlloc(NPAGES - 1);//错误：写入访问权限冲突，原因：申请了k页内存导致用尽后越界访问
    bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
    bigSpan->_n = NPAGES - 1;
    //将bigSpan挂接到NPAGES - 1号桶下
    _spanlists[bigSpan->_n].PushFront(bigSpan);
    //后面的逻辑仍为切分，即将bigSpan切分为更小的两块Span
    //则直接递归复用上述代码即可
    return NewPage(k);
}


// 由于切分span为小块内存后，将切分好的每一块内存的id都映射到了被切分的span上
//因此，通过该函数可以把所有连续的内存归还到其原来所在的span中
Span * PageCache::MapObjectToSpan(void* ptr)
{
    PAGE_ID id = (PAGE_ID)ptr >> PAGE_SHIFT;
    //加锁防止出现线程安全问题
    std::unique_lock<std::mutex> lock(_pageMtx);
    if (_idSpanMap.find(id) != _idSpanMap.end())
    {
        return _idSpanMap[id];
    }
    else
    {
        assert(false);
        return nullptr;
    }
}

void PageCache::RealeaseSpanToPageCache(Span* span)
{
    //若释放的span大于256KB，即span的页数大于NPAGES - 1，则直接将内存释放到堆
    if (span->_n > NPAGES - 1)
    {
        void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
        SystemFree(ptr);
        //delete span;
        _spanPool.Delete(span);
        return;
    }
    //对span前后的页进行回收，缓解内存碎片问题
    //1.处理span前的页
    while (1)
    {
        PAGE_ID prevId = span->_pageId - 1;
        auto ret = _idSpanMap.find(prevId);

        //如果span前没有页，那么无需再向前回收
        if (ret == _idSpanMap.end())
        {
            break;
        }

        Span* prevSpan = ret->second;
        //如果prevSpan正在被使用，那么无需再向前回收
        if (prevSpan->_isUse == true)
        {
            break;
        }

        //如果合并后的span页数超过NPAGES - 1，则无需再向前回收
        if (prevSpan->_n + span->_n > NPAGES - 1)
        {
            break;
        }

        //合并span前的页prevSpan
        span->_pageId = prevSpan->_pageId;
        span->_n += prevSpan->_n;
        //将prevSpan从其所挂接的桶中去除
        _spanlists[prevSpan->_n].Erase(prevSpan);
        //delete prevSpan;
        _spanPool.Delete(prevSpan);
    }
    //向后处理span后的页
    while (1)
    {
        PAGE_ID nextId = span->_pageId + span->_n;
        auto ret = _idSpanMap.find(nextId);

        //若后面的页不存在，则无需再向后回收
        if (ret == _idSpanMap.end())
        {
            break;
        }

        Span* nextSpan = ret->second;
        //若后面的页正在被使用，则无需再向后回收
        if (nextSpan->_isUse == true)
        {
            break;
        }

        //若后面的页合并后页数超过NAPGES - 1，则无需向后回收
        if (nextSpan->_n + span->_n > NPAGES - 1)
        {
            break;
        }

        //合并后面的页
        span->_n += nextSpan->_n;
        //将nextSpan从其所在的桶中取下来
        _spanlists[nextSpan->_n].Erase(nextSpan);
        //delete nextSpan;
        _spanPool.Delete(nextSpan);
    }
    //将span挂接到其所对应的Page下
    _spanlists[span->_n].PushFront(span);
    //标识span为未被使用的状态
    span->_isUse = false;
    //建立span的首尾PAGE_ID与span*的映射
    _idSpanMap[span->_pageId] = span;
    _idSpanMap[span->_pageId + span->_n - 1] = span;
}
