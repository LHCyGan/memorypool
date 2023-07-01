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

			// 获取一个k页的PageCache
			Span* NewPage(size_t k);
			//与CentralCache不同的是，若此处设置桶锁，会频繁的加锁解锁，故PageCache中整体设置一把锁
			std::mutex _pageMtx;

		private:
			PageCache() {}
			PageCache(const PageCache&) = delete;
			static PageCache _sInst;
			SpanList _spanlists[NPAGES];
		};
	}
}