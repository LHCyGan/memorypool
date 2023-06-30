#include "include/threadcache.h"

using namespace lh::mp;

void* ThreadCache::Allocate(size_t size) {
    assert(size <= MAX_BYTE);

    size_t alignSize = SizeClass::RoundUp(size);
    size_t index = SizeClass::Index(size);
    if (!_freeLists[index].Empty()) {
        return _freeLists[index].Pop();
    }
    else {
        return FetchFromCentralCache(index, alignSize);
    }
}

