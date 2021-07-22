#include "LFSKIPLIST.h"

bool Marked(LFNode *curr)
{
	LONG add = reinterpret_cast<LONG> (curr);
	return ((add & 0x1) == 0x1);
}

LFNode* GetReference(LFNode *curr)
{
	LONG addr = reinterpret_cast<LONG> (curr);
	return reinterpret_cast<LFNode *>(addr & 0xFFFFFFFE);
}

LFNode* Get(LFNode *curr, bool *marked)
{
	LONG addr = reinterpret_cast<LONG> (curr);
	*marked = ((addr & 0x01) != 0);
	return reinterpret_cast<LFNode *>(addr & 0xFFFFFFFE);
}

LFNode* AtomicMarkableReference(LFNode *node, bool mark)
{
	LONG addr = reinterpret_cast<LONG>(node);
	if (mark)
		addr = addr | 0x1;
	else
		addr = addr & 0xFFFFFFFE;
	return reinterpret_cast<LFNode *>(addr);
}

LFNode* Set(LFNode *node, bool mark)
{
	LONG addr = reinterpret_cast<LONG>(node);
	if (mark)
		addr = addr | 0x1;
	else
		addr = addr & 0xFFFFFFFE;
	return reinterpret_cast<LFNode *>(addr);
}


/***********************************/
// NODE
/***********************************/

LFNode::LFNode() {
	for (int i = 0; i<MAX_LEVEL; i++) {
		next[i] = AtomicMarkableReference(NULL, false);
	}
	topLevel = MAX_LEVEL;
}
LFNode::LFNode(int myKey) {
	key = myKey;
	for (int i = 0; i<MAX_LEVEL; i++) {
		next[i] = AtomicMarkableReference(NULL, false);
	}
	topLevel = MAX_LEVEL;
}
LFNode::LFNode(int x, int height) {
	key = x;
	for (int i = 0; i<MAX_LEVEL; i++) {
		next[i] = AtomicMarkableReference(NULL, false);
	}
	topLevel = height;
}
bool LFNode::CompareAndSet(int level, LFNode *old_node, LFNode *next_node, bool old_mark, bool next_mark) {
	LONG old_addr = reinterpret_cast<DWORD>(old_node);
	if (old_mark) old_addr = old_addr | 0x1;
	else old_addr = old_addr & 0xFFFFFFFE;
	LONG next_addr = reinterpret_cast<DWORD>(next_node);
	if (next_mark) next_addr = next_addr | 0x1;
	else next_addr = next_addr & 0xFFFFFFFE;
	int prev_addr = InterlockedCompareExchange(reinterpret_cast<long *>(&next[level]), next_addr, old_addr);
	return (prev_addr == old_addr);
}
/***********************************/
// SKIP_LIST
/***********************************/
LockFreeSkipList::LockFreeSkipList() {
	head = new LFNode(INT_MIN_VALUE);
	tail = new LFNode(INT_MAX_VALUE);
	for (int i = 0; i<MAX_LEVEL; i++) {
		head->next[i] = AtomicMarkableReference(tail, false);
	}
}

void LockFreeSkipList::Clear()
{
	LFNode *curr = head->next[0];
	while (curr != tail) {
		LFNode *temp = curr;
		curr = GetReference(curr->next[0]);
		delete temp;
	}
	for (int i = 0; i<MAX_LEVEL; i++) {
		head->next[i] = AtomicMarkableReference(tail, false);
	}
}

bool LockFreeSkipList::Find(int x, LFNode* preds[], LFNode* succs[])
{
	int bottomLevel = 0;
	bool marked = false;
	bool snip;
	LFNode* pred = NULL;
	LFNode* curr = NULL;
	LFNode* succ = NULL;
retry:
	while (true) {
		pred = head;
		for (int level = MAX_LEVEL - 1; level >= bottomLevel; level--) {
			curr = GetReference(pred->next[level]);
			while (true) {
				succ = curr->next[level];
				while (Marked(succ)) {
					snip = pred->CompareAndSet(level, curr, succ, false, false);
					if (!snip) goto retry;
					//	if (level == bottomLevel) freelist.free(curr);
					curr = GetReference(pred->next[level]);
					succ = curr->next[level];
				}
				if (curr->key < x) {
					pred = curr;
					curr = GetReference(succ);
				}
				else {
					break;
				}
			}
			preds[level] = pred;
			succs[level] = curr;
		}
		return (curr->key == x);
	}
}

bool LockFreeSkipList::Add(int x)
{
	int topLevel = rand() % MAX_LEVEL;
	int bottomLevel = 0;
	LFNode *preds[MAX_LEVEL + 1];
	LFNode *succs[MAX_LEVEL + 1];
	while (true) {
		bool found = Find(x, preds, succs);
		if (found) {
			return false;
		}
		else {
			LFNode* newNode = new LFNode(x, topLevel);
			for (int level = bottomLevel; level <= topLevel; level++) {
				LFNode* succ = succs[level];
				newNode->next[level] = Set(succ, false);
			}
			LFNode* pred = preds[bottomLevel];
			LFNode* succ = succs[bottomLevel];

			newNode->next[bottomLevel] = Set(succ, false);

			if (!pred->CompareAndSet(bottomLevel, succ, newNode, false, false))
				continue;

			for (int level = bottomLevel + 1; level <= topLevel; level++) {
				while (true) {
					pred = GetReference(preds[level]);
					succ = GetReference(succs[level]);
					if (pred->CompareAndSet(level, succ, newNode, false, false))
						break;
					Find(x, preds, succs);
				}
			}
			return true;
		}
	}
}

bool LockFreeSkipList::Remove(int x)
{
	int bottomLevel = 0;
	LFNode *preds[MAX_LEVEL + 1];
	LFNode *succs[MAX_LEVEL + 1];
	LFNode* succ;

	while (true) {
		bool found = Find(x, preds, succs);
		if (!found) return false;
		else {
			LFNode* nodeToRemove = succs[bottomLevel];
			for (int level = nodeToRemove->topLevel; level >= bottomLevel + 1; level--) {
				succ = nodeToRemove->next[level];
				while (!Marked(succ)) {
					nodeToRemove->CompareAndSet(level, succ, succ, false, true);
					succ = nodeToRemove->next[level];
				}
			}
			bool marked = false;
			succ = nodeToRemove->next[bottomLevel];
			while (true) {
				bool iMarkedIt = nodeToRemove->CompareAndSet(bottomLevel, succ, succ, false, true);
				succ = succs[bottomLevel]->next[bottomLevel];

				if (iMarkedIt) {
					Find(x, preds, succs);
					return true;
				}
				else if (Marked(succ)) return false;
			}
		}
	}
}

bool LockFreeSkipList::Contains(int x)
{
	int bottomLevel = 0;
	bool marked = false;
	LFNode *pred = head;
	LFNode *curr = NULL;
	LFNode *succ = NULL;

	for (int level = MAX_LEVEL - 1; level >= bottomLevel; level--) {
		curr = GetReference(pred->next[level]);
		while (true) {
			succ = curr->next[level];
			while (Marked(succ)) {
				curr = GetReference(curr->next[level]);
				succ = curr->next[level];
			}
			if (curr->key < x) {
				pred = curr;
				curr = GetReference(succ);
			}
			else {
				break;
			}
		}
	}
	return (curr->key == x);
}