//
// Created by Dell on 2023/6/29.
//

#include "include/objectpool.h"
using namespace lh::mp;

struct TreeNode
{
	int _val;
	TreeNode* _left;
	TreeNode* _right;
	TreeNode()
		:_val(0)
		, _left(nullptr)
		, _right(nullptr)
	{}
};

void TestObjectPool()
{
	// 申请释放的轮次
	const size_t Rounds = 5;
	// 每轮申请释放多少次
	const size_t N = 1000000;
	size_t begin1 = clock();
	std::vector<TreeNode*> v1;
	v1.reserve(N);

	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v1.push_back(new TreeNode);
		}
		for (int i = 0; i < N; ++i)
		{
			delete v1[i];
		}
		v1.clear();
	}
	size_t end1 = clock();
	ObjectPool<TreeNode> TNPool;
	size_t begin2 = clock();
	std::vector<TreeNode*> v2;
	v2.reserve(N);
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v2.push_back(TNPool.New());
		}
		for (int i = 0; i < 100000; ++i)
		{
			TNPool.Delete(v2[i]);
		}
		v2.clear();
	}
	size_t end2 = clock();
	std::cout << "new cost time:" << end1 - begin1 << std::endl;
	std::cout << "object pool cost time:" << end2 - begin2 << std::endl;
    std::cout << "111111111" << std::endl;
}

int main() {
    TestObjectPool();

    return 0;
}