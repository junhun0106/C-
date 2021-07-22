#pragma once

#include "stdafx.h"

const auto MAX_LEVEL = 11;
const auto INT_MAX_VALUE = 0x7FFFFFFF;
const auto INT_MIN_VALUE = 0x80000000;

class LFNode
{
public:
	int key;
	LFNode* next[MAX_LEVEL];
	int topLevel;

	LFNode();
	LFNode(int myKey);
	LFNode(int x, int height);
	bool CompareAndSet(int level, LFNode *old_node, LFNode *next_node, bool old_mark, bool next_mark);

};

class LockFreeSkipList
{
public:

	LFNode* head;
	LFNode* tail;

	LockFreeSkipList();

	LFNode* begin() { return head; }
	LFNode* end() { return tail; }

	void Clear();

	bool Find(int x, LFNode* preds[], LFNode* succs[]);

	bool Add(int x);

	bool Remove(int x);

	bool Contains(int x);
};