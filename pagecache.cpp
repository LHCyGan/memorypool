#include "pagecache.h"

PageCache PageCache::_sInst;

Span* PageCache::NewPage(size_t k) {
	assert(k && k < NPAGES);

	//若当前k页Page中有空闲的Span，返回
	if (!_spanlists[k].Empty())
		return _spanlists[k].PopFront();

	//若当前k页Page中无空闲的Span，则向后寻找更大页的Page去切分出需要的Span来
	for (size_t i = k + 1; i < NPAGES; i++) {
		if (!_spanlists[i].Empty()) {
			//若i页Span非空，则将其切分
			Span* kSpan = new Span;
			Span* nSpan = _spanlists[i].PopFront();

			//将nPage切分成k页Span及n - k页Span
			kSpan->_pageId = nSpan->_pageId;
			kSpan->_n = k;

			nSpan->_pageId += k;//细节错误：+=k写出-=k导致越界访问
			nSpan->_n -= k;
			//将切分后的nPage挂接到n-k页的Span下
			_spanlists[i - k].PushFront(nSpan);
			//返回切好的kSpan
			return kSpan;
		}
	}

	//走到此处说明PageCache中没有可以使用的Span了
	//则需要向系统（堆）申请一大块内存(128页)以备用
	Span* bigSpan = new Span;

	void* ptr = SystemAlloc(NPAGES - 1);//错误：写入访问权限冲突，原因：申请了k页内存导致用尽后越界访问
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->_n = NPAGES - 1;
	//将bigSpan挂接到NPAGES - 1号桶下
	_spanlists[bigSpan->_n].PushFront(bigSpan);
	//后面的逻辑仍为切分，即将bigSpan切分为更小的两块Span
	//则直接递归复用上述代码即可
	return NewPage(k);
}
