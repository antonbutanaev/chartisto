#include <memory>
#include <gtest/gtest.h>
#include <util/funcIterator.h>

using namespace std;
using namespace util;

TEST(TestFuncIterator, Range) {
	vector<int> v;
	iterateFunc(funcRangeIterator(10,15), [&](auto n){v.push_back(n);});
	decltype(v) v1{10,11,12,13,14};
	EXPECT_EQ(v, v1);
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
	{
		vector<string> s{"A", "B"};
		vector<pair<string, int>> v;
		iterateFunc(
			funcPairIterator(funcIterator(s), funcRangeIterator(1,3)),
			[&](auto n){v.push_back(n);}
		);
		decltype(v) v1 = {
			{"A",1},
			{"B",1},
			{"A",2},
			{"B",2},
		};
		EXPECT_EQ(v, v1);
	}
	{
		vector<string> s{"A", "B"};
		vector<pair<int, string>> v;
		iterateFunc(
			funcPairIterator(funcRangeIterator(1,3), funcIterator(s)),
			[&](auto n){v.push_back(n);}
		);
		decltype(v) v1 = {
			{1, "A"},
			{2, "A"},
			{1, "B"},
			{2, "B"},
		};
		EXPECT_EQ(v, v1);
	}
}

