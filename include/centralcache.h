#pragma once

#include "common.h"
using namespace lh::mp;

namespace lh {
	namespace mp {
		class CentralCache {
		public:
			static CentralCache* GetInstance() {
				return &_sInst;
			}

			// ��ȡһ���ǿ�Span;
			Span* GetOneSpan(SpanList& list, size_t byte_size);

			// ��CentralCache��ȡһ���������ڴ�����ThreadCache
			size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);

			// ��һ�������Ķ����ͷŵ�span���
			void ReleaseListToSpan(void* start, size_t size);

		private:
			SpanList _spanLists[NFREELIST]; //CentralCache��span�ָʽ��ThreadCache��ͬ

		private:
			CentralCache() {}
			CentralCache(const CentralCache&) = delete;//���ÿ���
			static CentralCache _sInst;//����ģʽ
		};

	}
}