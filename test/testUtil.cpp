#include <iostream>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <gtest/gtest.h>
#include <util/funcIterator.h>
#include <util/hasher.h>

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
	string a(10, 'A');
	string b(20, 'B');
	string c(30, 'C');
	int d = 40;
	char e = 'e';

	Hasher<string> hasher;

	auto seed = hasher(a);
	hashCombine(seed, b);
	hashCombine(seed, c);
	hashCombine(seed, d);
	hashCombine(seed, e);

	const auto seed2 = makeHash(e, d, c, b ,a);

	EXPECT_EQ(seed, seed2);
}

struct El {
	string s1, s2;
	bool operator==(const El &other) const {
		++numEq;
		return s1 == other.s1 && s2 == other.s2;
	}

	auto hash() const {
		++numHash;
		return makeHash(s1, s2);
	}

	static int numEq;
	static int numHash;
};

int El::numEq;
int El::numHash;

TEST(TestHash, StringHash) {
	uset<El> s;
	vector<El> v;
	const size_t num = 100000;
	for (size_t i = 0; i < num; ++i) {
		El el{"A" + to_string(i*2), "B" + to_string(10000000 + i*i)};
		v.push_back(el);
		s.insert(move(el));
	}

	cout << El::numEq << ' ' << El::numHash << endl;
	EXPECT_EQ(El::numEq, 0);
	EXPECT_EQ(El::numHash, num);

	El::numEq = 0;
	El::numHash = 0;

	size_t numFound = 0;
	for (const auto &el: v)
		if (s.find(el) != s.end())
			++numFound;

	cout << numFound << ' ' << El::numEq << ' ' << El::numHash << endl;

	EXPECT_EQ(numFound, num);
	EXPECT_EQ(El::numEq, num);
	EXPECT_EQ(El::numHash, num);
};
