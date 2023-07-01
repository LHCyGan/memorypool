#define _CRT_SECURE_NO_WARNINGS 1

#include "objectPool.h"
#include "concurrentalloc.h"

using namespace lh::mp;



void Alloc1()
{
	for (size_t i = 0; i < 5; i++)
	{
		ConcurrentAlloc(3);
		std::cout << std::this_thread::get_id() << ":" << pTLSThreadCache << std::endl;
	}
}

void Alloc2()
{
	for (size_t i = 0; i < 5; i++)
	{
		ConcurrentAlloc(6);
		std::cout << std::this_thread::get_id() << ":" << pTLSThreadCache << std::endl;
	}
}

void TLSTest()
{
	std::thread t1(Alloc1);
	t1.join();

	std::thread t2(Alloc2);
	t2.join();
}

void TestConcurrentAlloc1()
{
	void* p1 = ConcurrentAlloc(6);
	void* p2 = ConcurrentAlloc(8);
	void* p3 = ConcurrentAlloc(7);
	void* p4 = ConcurrentAlloc(1);
	void* p5 = ConcurrentAlloc(2);
	void* p6 = ConcurrentAlloc(3);
	std::cout << p1 << std::endl;
	std::cout << p2 << std::endl;
	std::cout << p3 << std::endl;
	std::cout << p4 << std::endl;
	std::cout << p5 << std::endl;
	std::cout << p6 << std::endl;
}

void TestConcurrentAlloc2()
{
	for (size_t i = 0; i < 1024; i++)
	{
		void* p1 = ConcurrentAlloc(8);
		std::cout << i << ":" << p1 << std::endl;
	}
	void* p2 = ConcurrentAlloc(8);
	std::cout << p2 << std::endl;
}

int main()
{
	//TestObjectPool();
	//TLSTest();
	//TestConcurrentAlloc1();
	TestConcurrentAlloc2();
	return 0;
}