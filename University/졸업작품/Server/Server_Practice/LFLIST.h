
#pragma once

#include "stdafx.h"

class CLFNODE {
public:
	int m_nKey;
	int m_nNext_and_marked;
public:
	CLFNODE();
	CLFNODE(int key);
	~CLFNODE(){}
	void SetKey(int key);
	void SetNext(CLFNODE *node);
	CLFNODE* GetNextAddr();
	CLFNODE* GetNextAddr_Makred(bool *marked);
	bool CAS(int old_value, int new_value);
	bool CAS(CLFNODE *old_node, CLFNODE *new_node, bool old_mark, bool new_mark);
	bool TryPOP(CLFNODE *next);
};


class CLFLIST
{
	CLFNODE head, tail;
public:
	CLFLIST();
	~CLFLIST();
	CLFNODE* begin() { return &head; }
	CLFNODE* end() { return &tail; }
	void InitializeLIST();
	void Find(CLFNODE **pred, CLFNODE **curr, int key);
	bool Add(int key);
	bool Remove(int key);
	bool Contains(int key);
};

