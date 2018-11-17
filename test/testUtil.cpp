#include <iostream>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <gtest/gtest.h>
#include <util/funcIterator.h>
#include <util/hashes.h>

using namespace std;
using namespace util;

TEST(TestFuncIterator, Range) {
	vector<int> v;
	iterateFunc(funcRangeIterator(10,15), [&](auto n){v.push_back(n);});
	decltype(v) v1{10,11,12,13,14};
	EXPECT_EQ(v, v1);
}

TEST(TestFuncIterator, Container) {
	vector<int> a{1,2,3};
	vector<int> b;
	iterateFunc(
		funcIterator(a),
		[&](auto n){b.push_back(n);}
	);
	vector<int> c{1,2,3};
	EXPECT_EQ(b, c);
}

TEST(TestFuncIterator, ContainerPart) {
	struct X{int a,b;};
	vector<X> a{{1,10},{2,20},{3, 30}};
	vector<int> b;
	iterateFunc(
		funcIterator(a, [](const auto &it){return &it.b;}),
		[&](auto n){b.push_back(n);}
	);
	vector<int> c{10,20,30};
	EXPECT_EQ(b, c);
}

TEST(TestFuncIterator, Transform) {
	vector<int> a{1,2,3};
	vector<int> b;
	iterateFunc(
		funcIteratorTransform(a, [](const auto &it){return 10*it;}),
		[&](auto n){b.push_back(n);}
	);
	vector<int> c{10,20,30};
	EXPECT_EQ(b, c);
}

TEST(TestFuncIterator, Pair) {
	{
		vector<pair<int, int>> v;
		iterateFunc(
			funcPairIterator(funcRangeIterator(1,3), funcRangeIterator(1,3)),
			[&](auto n){v.push_back(n);}
		);
		decltype(v) v1 = {
			{1,1},
			{2,1},
			{1,2},
			{2,2},
		};
		EXPECT_EQ(v, v1);
	}
	{
		vector<pair<int, int>> v;
		iterateFunc(
			funcPairIterator(funcRangeIterator(1,2), funcRangeIterator(1,3)),
			[&](auto n){v.push_back(n);}
		);
		decltype(v) v1 = {
			{1,1},
			{1,2},
		};
		EXPECT_EQ(v, v1);
	}
	{
		vector<pair<int, int>> v;
		iterateFunc(
			funcPairIterator(funcRangeIterator(1,1), funcRangeIterator(1,3)),
			[&](auto n){v.push_back(n);}
		);
		EXPECT_TRUE(v.empty());
	}
	{
		vector<pair<int, int>> v;
		iterateFunc(
			funcPairIterator(funcRangeIterator(1,3), funcRangeIterator(2,3)),
			[&](auto n){v.push_back(n);}
		);
		decltype(v) v1 = {
			{1,2},
			{2,2},
		};
		EXPECT_EQ(v, v1);
	}
	{
		vector<pair<int, int>> v;
		iterateFunc(
			funcPairIterator(funcRangeIterator(1,3), funcRangeIterator(3,3)),
			[&](auto n){v.push_back(n);}
		);
		EXPECT_TRUE(v.empty());
	}

	const string A(20, 'A');
	const string B(20, 'B');

	{
		vector<string> s{A, B};
		vector<pair<string, int>> v;
		iterateFunc(
			funcPairIterator(funcIterator(s), funcRangeIterator(1,3)),
			[&](auto n){v.push_back(n);}
		);
		decltype(v) v1 = {
			{A, 1},
			{B, 1},
			{A, 2},
			{B, 2},
		};
		EXPECT_EQ(v, v1);
	}
	{
		vector<string> s{A, B};
		vector<pair<int, string>> v;
		iterateFunc(
			funcPairIterator(funcRangeIterator(1,3), funcIterator(s)),
			[&](auto n){v.push_back(n);}
		);
		decltype(v) v1 = {
			{1, A},
			{2, A},
			{1, B},
			{2, B},
		};
		EXPECT_EQ(v, v1);
	}
	{
		vector<pair<int, pair<int, int>>> v;
		iterateFunc(
			funcPairIterator(
				funcRangeIterator(1,3),
				funcPairIterator(funcRangeIterator(1,3), funcRangeIterator(1,3))
			),
			[&](auto n){v.push_back(n);}
		);
		decltype(v) v1 = {
			{1, {1, 1}},
			{2, {1, 1}},
			{1, {2, 1}},
			{2, {2, 1}},
			{1, {1, 2}},
			{2, {1, 2}},
			{1, {2, 2}},
			{2, {2, 2}},
		};
		EXPECT_EQ(v, v1);
	}
}

TEST(TestHash, MakeHash) {
	string a(20, 'A');
	string b(20, 'B');
	string c(20, 'C');

	Hasher<string> hasher;

	auto seed = hasher(a);
	hashCombine(seed, b);
	hashCombine(seed, c);

	const auto seed2 = makeHash(c,b,a);

	EXPECT_EQ(seed, seed2);
}
