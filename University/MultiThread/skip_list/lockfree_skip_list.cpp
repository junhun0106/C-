#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <string>

#define NUM_TEST   4000000
#define KEY_RANGE  1000

#define THREAD_MAX 32

int num_thread;

#define MAX_LEVEL		11
#define INT_MAX_VALUE	0x7FFFFFFF
#define INT_MIN_VALUE	0x80000000

using namespace std;

class LFNode;


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

class LFNode
{
public:
	int key;
	LFNode* next[MAX_LEVEL];
	int topLevel;

	// 보초노드에 관한 생성자
	LFNode() {
		for (int i = 0; i<MAX_LEVEL; i++) {
			next[i] = AtomicMarkableReference(NULL, false);
		}
		topLevel = MAX_LEVEL;
	}
	LFNode(int myKey) {
		key = myKey;
		for (int i = 0; i<MAX_LEVEL; i++) {
			next[i] = AtomicMarkableReference(NULL, false);
		}
		topLevel = MAX_LEVEL;
	}

	// 일반노드에 관한 생성자
	LFNode(int x, int height) {
		key = x;
		for (int i = 0; i<MAX_LEVEL; i++) {
			next[i] = AtomicMarkableReference(NULL, false);
		}
		topLevel = height;
	}

	void InitNode() {
		key = 0;
		for (int i = 0; i<MAX_LEVEL; i++) {
			next[i] = AtomicMarkableReference(NULL, false);
		}
		topLevel = MAX_LEVEL;
	}

	void InitNode(int x, int height) {
		key = x;
		for (int i = 0; i<MAX_LEVEL; i++) {
			next[i] = AtomicMarkableReference(NULL, false);
		}
		topLevel = height;
	}

	bool CompareAndSet(int level, LFNode *old_node, LFNode *next_node, bool old_mark, bool next_mark) {
		LONG old_addr = reinterpret_cast<DWORD>(old_node);
		if (old_mark) old_addr = old_addr | 0x1;
		else old_addr = old_addr & 0xFFFFFFFE;
		LONG next_addr = reinterpret_cast<DWORD>(next_node);
		if (next_mark) next_addr = next_addr | 0x1;
		else next_addr = next_addr & 0xFFFFFFFE;
		int prev_addr = InterlockedCompareExchange(reinterpret_cast<long *>(&next[level]), next_addr, old_addr);
		return (prev_addr == old_addr);
	}

	bool AttemptMark(LFNode *old_node, bool mark)
	{
		LONG old_addr = reinterpret_cast<LONG>(old_node);
		LONG new_addr = old_addr;
		if (mark)
			new_addr = new_addr | 0x1;
		else
			new_addr = new_addr & 0xFFFFFFFE;
		LONG prev_addr = InterlockedCompareExchange(reinterpret_cast<long *> (&next), new_addr, old_addr);
		return (prev_addr == old_addr);
	}
};

class FREELIST {
	LFNode* first;
	LFNode* second;
public:
	FREELIST() {
		first = new LFNode(0);
		second = new LFNode(0);
	}
	~FREELIST() {
		while (first != nullptr) {
			LFNode* temp = first;
			first = first->next[0];
			delete temp;
		}
		while (second != nullptr) {
			LFNode* temp = second;
			second = second->next[0];
			delete second;
		}
	}
	void ExChange() {
		LFNode* temp = first;
		first = second;
		second = first;
	}
	LFNode* GetNode(int value, int height) {
		//ExChange();
		while (true) {
			LFNode* head = first;
			LFNode* next = head->next[0];
			if (nullptr == next) return (new LFNode(value, height));

			if (false == first->CompareAndSet(0, head, next, true, false)) continue;
			
			head->key = value;
			head->topLevel = height;

			for (auto i = 0; i < height; ++i) {
				head->next[i] = nullptr;
			}

			return head;
		}
	}
	void FreeNode(LFNode *free_node) {
		while (true) {
			LFNode* head = second;
			free_node->next[0] = head;
			if (false == second->CompareAndSet(0, head, free_node, false, true)) continue;
			return;
		}
	}
};

FREELIST free_list;
class LockFreeSkipList
{
public:

	LFNode* head;
	LFNode* tail;

	LockFreeSkipList() {
		head = new LFNode(INT_MIN_VALUE);
		tail = new LFNode(INT_MAX_VALUE);
		for (int i = 0; i<MAX_LEVEL; i++) {
			head->next[i] = AtomicMarkableReference(tail, false);
		}
	}

	void Clear()
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

	bool Find(int x, LFNode* preds[], LFNode* succs[])
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
					while (Marked(succ)) { //표시되었다면 제거
						snip = pred->CompareAndSet(level, curr, succ, false, false);
						if (!snip) goto retry;
						if (level == bottomLevel) free_list.FreeNode(curr);
						curr = GetReference(pred->next[level]);
						succ = curr->next[level];
					}

					// 표시 되지 않은 경우
					// 키값이 현재 노드의 키값보다 작다면 pred전진
					if (curr->key < x) {
						pred = curr;
						curr = GetReference(succ);
						// 키값이 그렇지 않은 경우
						// curr키는 대상키보다 같거나 큰것이므로 pred의 키값이 
						// 대상 노드의 바로 앞 노드가 된다.		
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

	bool Add(int x)
	{
		int topLevel = rand() % MAX_LEVEL;
		int bottomLevel = 0;
		LFNode *preds[MAX_LEVEL + 1];
		LFNode *succs[MAX_LEVEL + 1];
		while (true) {
			bool found = Find(x, preds, succs);
			// 대상 키를 갖는 표시되지 않은 노드를 찾으면 키가 이미 집합에 있으므로 false 반환
			if (found) {
				return false;
			}
			else {
				LFNode* newNode = free_list.GetNode(x, topLevel);
				//newNode->InitNode(x, topLevel);

				for (int level = bottomLevel; level <= topLevel; level++) {
					LFNode* succ = succs[level];
					// 현재 새노드의 next는 표시되지 않은 상태, find()가 반환반 노드를 참조
					newNode->next[level] = Set(succ, false);
				}

				//find에서 반환한 pred와 succ의 가장 최하층을 먼저 연결
				LFNode* pred = preds[bottomLevel];
				LFNode* succ = succs[bottomLevel];

				newNode->next[bottomLevel] = Set(succ, false);

				//pred->next가 현재 succ를 가리키고 있는지 않았는지 확인하고 newNode와 참조설정
				if (!pred->CompareAndSet(bottomLevel, succ, newNode, false, false))
					// 실패일경우는 next값이 변경되었으므로 다시 호출을 시작
					continue;

				for (int level = bottomLevel + 1; level <= topLevel; level++) {
					while (true) {
						pred = GetReference(preds[level]);
						succ = GetReference(succs[level]);
						// 최하층 보다 높은 층들을 차례대로 연결
						// 연결을 성공할경우 다음단계로 넘어간다
						if (pred->CompareAndSet(level, succ, newNode, false, false))
							break;
						//Find호출을 통해 변경된 preds, succs를 새로 얻는다.
						Find(x, preds, succs);
					}
				}

				//모든 층에서 연결되었으면 true반환
				return true;
			}
		}
	}

	bool Remove(int x)
	{
		int bottomLevel = 0;
		LFNode *preds[MAX_LEVEL + 1];
		LFNode *succs[MAX_LEVEL + 1];
		LFNode* succ;

		while (true) {
			bool found = Find(x, preds, succs);
			if (!found) {
				//최하층에 제거하려는 노드가 없거나, 짝이 맞는 키를 갖는 노드가 표시 되어 있다면 false반환
				return false;
			}
			else {
				LFNode* nodeToRemove = succs[bottomLevel];
				//최하층을 제외한 모든 노드의 next와 mark를 읽고 AttemptMark를 이용하여 연결에 표시
				for (int level = nodeToRemove->topLevel; level >= bottomLevel + 1; level--) {
					succ = nodeToRemove->next[level];
					// 만약 연결이 표시되어있으면 메서드는 다음층으로 이동
					// 그렇지 않은 경우 다른 스레드가 병행을 햇다는 뜻이므로 현재 층의 연결을 다시 읽고
					// 연결에 다시 표시하려고 시도한다.
					while (!Marked(succ)) {
						nodeToRemove->CompareAndSet(level, succ, succ, false, true);
						succ = nodeToRemove->next[level];
					}
				}
				//이부분에 왔다는 것은 최하층을 제외한 모든 층에 표시했다는 의미

				bool marked = false;
				succ = nodeToRemove->next[bottomLevel];
				while (true) {
					//최하층의 next참조에 표시하고 성공했으면 Remove()완료
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

	bool Contains(int x)
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
	void printfirst20()
	{
		LFNode *curr = head;
		printf("First 20 entries are : ");
		for (int i = 0; i<20; ++i) {
			curr = curr->next[0];
			if (NULL == curr) break;
			printf("%d(%d), ", curr->key, curr->topLevel);
		}
		printf("\n");
	}
};

LockFreeSkipList LockFree;

DWORD WINAPI ThreadFunc(void *lpVoid)
{
	int key;

	for (int i = 0; i < NUM_TEST / num_thread; i++) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			LockFree.Add(key);
			break;
		case 1: key = rand() % KEY_RANGE;
			LockFree.Remove(key);
			break;
		case 2: key = rand() % KEY_RANGE;
			LockFree.Contains(key);
			break;
		default: printf("Error\n");
			exit(-1);

		}
	}
	return 0;
}



int main()
{
	DWORD addr;
	HANDLE hThread[THREAD_MAX];
	LARGE_INTEGER	end, start, freq;
	QueryPerformanceFrequency(&freq);

	for (num_thread = 1; num_thread <= 64; num_thread *= 2) {
		LockFree.Clear();
		free_list.ExChange();
		QueryPerformanceCounter(&start);
		for (int i = 0; i<num_thread; i++)
			hThread[i] = CreateThread(NULL, 0, ThreadFunc, (LPVOID)i, 0, &addr);
		for (int i = 0; i<num_thread; i++)
			WaitForSingleObject(hThread[i], INFINITE);
		QueryPerformanceCounter(&end);
		for (int i = 0; i<num_thread; i++)
			CloseHandle(hThread[i]);
		printf("%dThread Time : %g seconds\n", num_thread, (double)(end.QuadPart - start.QuadPart) / freq.QuadPart);
		LockFree.printfirst20();
	}

	getchar();

	return 0;
}