#pragma once

#include "common.h"
#include "threadcache.h"

using namespace lh::mp;

namespace lh {
	namespace mp {
		void* ConcurrentAlloc(size_t size) {
			//每个线程通过TLS各自创建自己私有的ThreadCache对象
			if (pTLSThreadCache == nullptr) {
				pTLSThreadCache = new ThreadCache;
			}
			return pTLSThreadCache->Allocate(size);
		}

		void ConcurrentFree(void* ptr, size_t size) {
			assert(pTLSThreadCache);
			pTLSThreadCache->DeAllocate(ptr, size);
		}

	}

}