#define _CRT_SECURE_NO_WARNINGS 1

#include "threadcache.h"
#include "centralcache.h"

using namespace lh::mp;

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size) {
	//Ԥ�ƻ�ȡ����������
	//�����������㷨
	//1���տ�ʼ��CentralCache��ȡ������������С������̫������ò���
	//2����size����������ʱ��batchNum�Ͳ�������ֱ������
	//3��sizeԽС��һ����CentralCache��������������ͽϴ�
	//4��sizeԽ��һ����CentralCache��������������ͽ�С
	size_t batchNum = min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));
	if (_freeLists[index].MaxSize() == batchNum)
		_freeLists[index].MaxSize() += 1;
	void* start = nullptr;
	void* end = nullptr;

	//��CentralCache����һ���������ڴ����
	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	assert(actualNum);

	if (actualNum == 1)
		assert(start == end);
	else
		_freeLists[index].PushRange(NextObj(start), end);

	return start;
}


void* ThreadCache::Allocate(size_t size) {
	assert(size <= MAX_BYTE);

	size_t alignSize = SizeClass::RoundUp(size);//����Ҫ������ڴ�ռ��С
	size_t index = SizeClass::Index(size);//���������������е�����

	if (!_freeLists[index].Empty()) {
		return _freeLists[index].Pop();//��������ǿգ�����ȡ�ڴ�
	}
	else {
		return FetchFromCentralCache(index, alignSize);//���������Ļ�������
	}
}

void ThreadCache::DeAllocate(void* ptr, size_t size) {
	assert(ptr);
	assert(size <= MAX_BYTE);

	size_t index = SizeClass::Index(size);

	//�ҵ���Ӧ����������Ͱ�����������
	_freeLists[index].Push(ptr);
}

