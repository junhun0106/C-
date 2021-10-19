/*
2016년 6월 2일 목요일
전공 특강
잠금 기반의 병행 스킵 리스트

lazy skip_list

- non-blocking skip_list에 전 단계

- 노드
:: 개별적인 잠금
:: Marked 필드(bool) - remove시 논리적으로 제거하고 있는 중이라면 true
:: next - 참조의 배열로서 자신이 속한 각 리스트에 대한 참조
:: 보초노드(head,tail) - 초기(skiplist가 비어 있을 경우) 모든 층에서 head는 tail의 앞 노드 이다.

- Add()
:: add는 항상 find를 호출 - find() 스킵리스트를 순회하고 모든 층에 대해 앞 노드와 뒤 노드를 반환.
:: 노드가 추가되는 동안 앞 노드가 변경되는 것을 방지하기 위해 앞 노드들을 잠근다. -> pred, curr가 여러 개이기 때문에 앞 노드들이 바뀌는 것을 방지 해야 한다.
:: fully linked 필드 -> 연결 된 노드들이 한 큐에 다 들어가지 않아, 덜 들어간 상태가 생길 수 있다.
	- 모든 층에서 추가된 노드에 제대로 참조를 설정 할 때 까지 논리적으로 집합에 있지 않다고 판단 
		-> 몽땅 다 연결이 楹?안楹じ?저장 해야 한다.
	- 모든 층에 연결 될 경우에 true
	- false일 경우에 접근이 허용되지 않으며 true가 될 때 까지 spin
		-> true가 되지 않으면, 모드 필드가 다 채워지지 않은 경우 이므로, 건들면 안된다. (없는 셈 친다)
		-> 다른 스레드가 풀 링크드 필드를 발견 했을 경우 풀 링크드가 해결 될 때 까지 기다려야 한다.

- Remove()
:: Find를 호출해서 대상키를 가지고 있는 노드가 이미 리스트에 있는 확인
	-> 리스트에 있다면 제거할 노드를 삭제 할 수 있는지 확인
	-> Marked, fully_linked 필드 이용해서 확인
	-> 삭제 할 수 있다면, 마크 필드를 설정하여 논리적으로 삭제
	-> 물리적인 삭제 방법은 -> 대상 노드의 앞 노드와 대상노드를 잠그고, 각 층에서 하나씩 대상 노드를 제거(위층 부터 제거)하고 다음 노드와 연결 한다.

*/

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <vector>
#include <Windows.h>

using namespace std;

const auto MAX_LEVEL = 10;
const auto MAX_THREAD = 64;
const auto NUM_TEST = 4000000;
const auto KEY_RANGE = 1000;

class LAZY_Node {
public:
	int key;
	LAZY_Node *next[MAX_LEVEL];
	int top_level;

	volatile bool marked;
	volatile bool fully_linked;

	recursive_mutex lnode_lock;

	LAZY_Node() {
		for (auto i = 0; i < MAX_LEVEL; ++i) next[i] = nullptr;
		top_level = MAX_LEVEL - 1; 
		marked = fully_linked = false;
	}
	LAZY_Node(int k) {
		key = k;
		for (auto i = 0; i < MAX_LEVEL; ++i) next[i] = nullptr;
		top_level = MAX_LEVEL - 1;
		marked = fully_linked = false;
	}
	LAZY_Node(int k, int height) {
		key = k;
		for (auto i = 0; i < MAX_LEVEL; ++i) next[i] = nullptr;
		top_level = height;
		marked = fully_linked = false;
	}
	~LAZY_Node() {

	}

	void lock() {
		lnode_lock.lock();
	}
	void unlock() {
		lnode_lock.unlock();
	}
};

class LAZY_SKIP_LIST {
	LAZY_Node head, tail;
public:
	LAZY_SKIP_LIST() {
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		for (auto i = 0; i < MAX_LEVEL; ++i)
			head.next[i] = &tail;
	}
	~LAZY_SKIP_LIST() {
		Init();
	}
	void Init() {
		LAZY_Node *ptr;
		while (head.next[0] != &tail) { 
			ptr = head.next[0];
			head.next[0] = head.next[0]->next[0];
			delete ptr;
		}
		for (auto i = 1; i < MAX_LEVEL; ++i) head.next[i] = &tail;
	}

	typedef LAZY_Node *Nexts[MAX_LEVEL];
	// find 찾은 레벨을 리턴해서, 수정 회수를 줄일 수 있다.
	
	// lazy에서는 파인드에서 찾은 마지막 레벨을 리턴 해야 한다.
	int Find(int key, Nexts* pred, Nexts* curr) {
		int last_found = -1;
		for (auto level = MAX_LEVEL - 1; level >= 0; --level) {
			if ((MAX_LEVEL - 1) == level) (*pred)[level] = &head;
			else (*pred)[level] = (*pred)[level + 1];

			(*curr)[level] = (*pred)[level]->next[level];

			while ((*curr)[level]->key < key) {
				(*pred)[level] = (*curr)[level];
				(*curr)[level] = (*curr)[level]->next[level];
			}
			if ((last_found = -1) && ((*curr)[level]->key == key)) last_found = level;
		}
		return last_found;
	}

	bool Add(int key) {
		Nexts pred, curr;
		while (true) {
			int lfound = Find(key, &pred, &curr);
			if (lfound != -1) { // 같은 키가 있어
				LAZY_Node *node_found = (curr)[lfound]; // lfound가 -1이 아니면, key가 같은 노드가 있다는 뜻. 그러면 false를 해줘야 한다.
				if (false == node_found->marked) { // marked가 false이고,
					while (false == node_found->fully_linked);// fully_linked가 true가 될 때 까지, 기다렸다가 add를 할 수 없다고 리턴해준다.
					return false;
				} 
				// 리스트에 없는거니까 유령존재를 발견했으니까 다시.
				continue;
				// marked가 되어있으면 없는 노드에 없으므로 다시 찾아야 한다.
			}
			int highest_locked = -1; // lock이 걸려있는 레벨 중 가장 높은 레벨.
			int height = 0;
			for (height; height < MAX_LEVEL - 1; ++height) if (rand() % 2 == 0) break;
			bool valid = true;
			LAZY_Node *pre, *succ;
			for (int level = 0; valid && (level <= height); ++level) {
				pre = pred[level];
				succ = curr[level];
				pre->lock();
				highest_locked = level;
				valid = (!pre->marked) && (!succ->marked) && (pre->next[level] == succ);
				if (!valid) break;
			} // validate 한지 확인 한다.
			if (!valid) { // 서로 다른 노드라 할지라도, 
				for (auto level = 0; level <= highest_locked; ++level)
					pred[level]->unlock(); // 락을 해제 하지 않으면, 서로 다른 레벨에서의 lock이 뒤엉켜버리기 때문에 해제 하여야 한다.
				continue;
			}
			LAZY_Node* node = new LAZY_Node(key, height);
			for (auto i = 0; i < height + 1; ++i) {
				node->next[i] = curr[i];
				pred[i]->next[i] = node;
			}
			node->fully_linked = true;
			for (auto level = 0; level <= highest_locked; ++level)
				pred[level]->unlock();
			return true;
		}
	}

	bool Remove(int key) {
		LAZY_Node *victim = nullptr;
		bool is_marked = false;
		int top_level = -1;
		Nexts pred, curr;
		while (true) {
			int lfound = Find(key, &pred, &curr);
			if (lfound != -1) victim = curr[lfound];
			if ((is_marked) | 
				(lfound != -1) &&
				(victim->fully_linked) && 
				((victim->top_level) == lfound) && 
				(!victim->marked)) {
				if (!is_marked) {
					top_level = victim->top_level;
					victim->lock();
					if (victim->marked) {
						victim->unlock();
						return false;
					}
					//victim->marked = true;
					is_marked = true;
				}
				int highest_locked = -1;
				LAZY_Node *pre; bool valid = true;
				for (int level = 0; valid && (level <= top_level); ++level) {
					pre = pred[level];
					pre->lock();
					highest_locked = level;
					valid = ((!pre->marked) && (pre->next[level] == victim));
					if (!valid) break;
				}
				if (!valid) {
					for (auto level = 0; level <= highest_locked; ++level)
						pred[level]->unlock();
					victim->unlock();
					continue;
				}
				// 여기서 lock을 풀어버리면, marked가 된 상태로 find에서 뒷수습을 해줘야 하는데
				// marked는 true인데 리스트에서 지워지지 않아서 add에서 먹통이 되어 버림.
				// 따라서 find에서 marked true가 된 것을 처리해주거나, 아니라면 true를 무조건 바꿔야하는 곳에서만 바꾸어주면 된다??
				for (auto level = top_level; level >= 0; --level) {
					victim->marked = true;
					pred[level]->next[level] = victim->next[level];
				}
				victim->unlock();
				for (int i = 0; i <= highest_locked; ++i) {
					pred[i]->unlock();
				}
				//delete curr[0];
				return true;
			}
			else return false;
		}
	}

	bool Contains(int key) {
		Nexts pred, curr;
		int lFound = Find(key, &pred, &curr);
		return ((lFound != -1)
			&& (curr[lFound]->fully_linked)
			&& (!curr[lFound]->marked));
	}

	void print20() {
		cout << "SKIP_LIST = ";
		LAZY_Node *ptr = head.next[0];
		for (auto i = 0; i < 20; ++i) {
			if (&tail == ptr) break;
			cout << ptr->key << "[" << ptr->top_level << "] : ";
			ptr = ptr->next[0];
		}
		cout << endl;
	}
};

LAZY_SKIP_LIST lsklist;

void test_thread_func(int threa_num) {
	int key;
	int count = 0;
	for (auto i = 0; i < NUM_TEST / threa_num; ++i) {
		switch (0) {
		case 0: key = rand() % KEY_RANGE;
			lsklist.Add(key); break;
		case 1: key = rand() % KEY_RANGE;
			lsklist.Remove(key); 
			break;
		case 2: key = rand() % KEY_RANGE;
			lsklist.Contains(key); 
			break;
		default: cout << "Error" << endl;
			exit(-1);
		}
	}
	//cout << "count : " << count << endl;
}
int main() {
	std::vector<std::thread*> v_threadlist;
	v_threadlist.reserve(8);
	for (auto i = 1; i <= MAX_THREAD; i *= 2) {
		auto start_time = std::chrono::high_resolution_clock::now();
		lsklist.Init();
		for (auto j = 0; j < i; ++j) {
			v_threadlist.push_back(new std::thread{ test_thread_func, i });
		}
		for each (auto th in v_threadlist)
		{
			th->join();
			delete th;
		}
		auto end_time = std::chrono::high_resolution_clock::now() - start_time;
		cout << "thread 개수 : " << i << endl;
		cout << std::chrono::duration_cast<std::chrono::milliseconds>(end_time).count() << "msecons" << endl;
		v_threadlist.clear();
		lsklist.print20();
	}
	system("pause");
}