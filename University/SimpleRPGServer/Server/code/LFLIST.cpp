#include "LFLIST.h"
CLFNODE::CLFNODE() {
	m_nNext_and_marked = 0;
}
CLFNODE::CLFNODE(int key) {
	m_nKey = key;
	m_nNext_and_marked = 0;
}
void CLFNODE::SetKey(int key) { m_nKey = key; }
void CLFNODE::SetNext(CLFNODE *node) {
	m_nNext_and_marked = reinterpret_cast<int>(node) & 0xFFFFFFFE;
}
CLFNODE* CLFNODE::GetNextAddr() {
	return reinterpret_cast<CLFNODE*>(m_nNext_and_marked & 0xFFFFFFFE);
}
CLFNODE* CLFNODE::GetNextAddr_Makred(bool *marked) {
	*marked = ((m_nNext_and_marked & 1) == 1);
	return reinterpret_cast<CLFNODE*>(m_nNext_and_marked & 0xfffffffe);
}
bool CLFNODE::CAS(int old_value, int new_value) {
	return atomic_compare_exchange_strong(reinterpret_cast<atomic<int>*>(&m_nNext_and_marked), &(old_value), new_value);
}
bool CLFNODE::CAS(CLFNODE *old_node, CLFNODE *new_node, bool old_mark, bool new_mark) {
	int old_value = reinterpret_cast<int>(old_node);
	if (old_mark) old_value = old_value | 1;
	else old_value = old_value & 0xfffffffe;

	int new_value = reinterpret_cast<int>(new_node);
	if (new_mark) new_value = new_value | 1;
	else new_value = new_value & 0xfffffffe;

	return CAS(old_value, new_value);

}
bool CLFNODE::TryPOP(CLFNODE *next) {
	int old_value = reinterpret_cast<int>(next) & 0xfffffffe;
	int new_value = old_value | 1;
	return CAS(old_value, new_value);
}

/*************************************************************************************/

CLFLIST::CLFLIST()
{
	head.m_nKey = 0x80000000;
	tail.m_nKey = 0x7fffffff;
	head.SetNext(&tail);
}
CLFLIST::~CLFLIST()
{
	InitializeLIST();
}

void CLFLIST::InitializeLIST() {
	CLFNODE *ptr;
	while (head.GetNextAddr() != &tail) {
		ptr = head.GetNextAddr();
		head.SetNext(head.GetNextAddr()->GetNextAddr());
		delete ptr;
	}
}
void CLFLIST::Find(CLFNODE **pred, CLFNODE **curr, int key) {
	CLFNODE *succ;
fail_retry:
	*pred = &head;
	*curr = (*pred)->GetNextAddr();

	while (true) {
		bool is_removed;
		succ = (*curr)->GetNextAddr_Makred(&is_removed);
		while (is_removed) {
			if (!(*pred)->CAS((*curr), succ, false, false))
				goto fail_retry;
			(*curr) = succ;
			succ = (*curr)->GetNextAddr_Makred(&is_removed);

		}
		if ((*curr)->m_nKey >= key) return;

		(*pred) = (*curr);
		(*curr) = succ;
	}
}
bool CLFLIST::Add(int key) {
	CLFNODE* pred, *curr;
	while (true) {
		Find(&pred, &curr, key);
		if (curr->m_nKey == key) return false;
		else {
			CLFNODE *node = new CLFNODE(key);
			node->SetNext(curr);
			if (pred->CAS(curr, node, false, false)) return true;
			else { delete node; continue; }
		}
	}
}
bool CLFLIST::Remove(int key) {
	CLFNODE *pred, *curr;
	bool snip;
	while (true) {
		Find(&pred, &curr, key);
		if (curr->m_nKey != key) { return false; }
		else {
			CLFNODE *succ = curr->GetNextAddr();
			snip = curr->TryPOP(succ);
			if (!snip) continue;
			pred->CAS(curr, succ, false, false);
			return true;
		}
	}
}
bool CLFLIST::Contains(int key) {
	bool is_removed;
	CLFNODE *curr = &head;
	while (curr->m_nKey < key) curr = curr->GetNextAddr_Makred(&is_removed);
	return (curr->m_nKey == key && !is_removed);
}
