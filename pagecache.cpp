#include "pagecache.h"

PageCache PageCache::_sInst;

Span* PageCache::NewPage(size_t k) {
	assert(k && k < NPAGES);

	//����ǰkҳPage���п��е�Span������
	if (!_spanlists[k].Empty())
		return _spanlists[k].PopFront();

	//����ǰkҳPage���޿��е�Span�������Ѱ�Ҹ���ҳ��Pageȥ�зֳ���Ҫ��Span��
	for (size_t i = k + 1; i < NPAGES; i++) {
		if (!_spanlists[i].Empty()) {
			//��iҳSpan�ǿգ������з�
			Span* kSpan = new Span;
			Span* nSpan = _spanlists[i].PopFront();

			//��nPage�зֳ�kҳSpan��n - kҳSpan
			kSpan->_pageId = nSpan->_pageId;
			kSpan->_n = k;

			nSpan->_pageId += k;//ϸ�ڴ���+=kд��-=k����Խ�����
			nSpan->_n -= k;
			//���зֺ��nPage�ҽӵ�n-kҳ��Span��
			_spanlists[i - k].PushFront(nSpan);
			//�����кõ�kSpan
			return kSpan;
		}
	}

	//�ߵ��˴�˵��PageCache��û�п���ʹ�õ�Span��
	//����Ҫ��ϵͳ���ѣ�����һ����ڴ�(128ҳ)�Ա���
	Span* bigSpan = new Span;

	void* ptr = SystemAlloc(NPAGES - 1);//����д�����Ȩ�޳�ͻ��ԭ��������kҳ�ڴ浼���þ���Խ�����
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	bigSpan->_n = NPAGES - 1;
	//��bigSpan�ҽӵ�NPAGES - 1��Ͱ��
	_spanlists[bigSpan->_n].PushFront(bigSpan);
	//������߼���Ϊ�з֣�����bigSpan�з�Ϊ��С������Span
	//��ֱ�ӵݹ鸴���������뼴��
	return NewPage(k);
}
