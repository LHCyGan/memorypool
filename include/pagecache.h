#pragma once

#include "common.h"
using namespace lh::mp;

namespace lh {
	namespace mp {
		class PageCache {
		public:
			static PageCache* GetInstance() {
				return &_sInst;
			}

			// ��ȡһ��kҳ��PageCache
			Span* NewPage(size_t k);
			//��CentralCache��ͬ���ǣ����˴�����Ͱ������Ƶ���ļ�����������PageCache����������һ����
			std::mutex _pageMtx;

		private:
			PageCache() {}
			PageCache(const PageCache&) = delete;
			static PageCache _sInst;
			SpanList _spanlists[NPAGES];
		};
	}
}