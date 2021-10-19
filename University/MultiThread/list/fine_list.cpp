/*
2016년 4월 7일 목요일
전공특강

CAS Compare and Set/Swap.

*/

// Lock - free 실습
// 1step. Blocking 자료구조 ->> 멀티스레드에서 돌아가지 않는다.
// 2step. 성긴 동기화(coarse-grained synchronization) ->> 전체에 락을 걸어보는 방법.
// 3step. 세밀한 동기화 (fine-grained synchronization) ->> 노드마다 락을 걸고 들어가는 방법.
// 4step.

#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

using namespace std;

class CNODE {
public:
	int key;
	mutex glock;
	CNODE *next;

public:
	CNODE() {
		next = nullptr;
	}
	CNODE(int num) {
		key = num;
		next = nullptr;
	}
	~CNODE() {

	}

	void lock() {
		glock.lock();
	}
	void unlock() {
		glock.unlock();
	}
};

class CSET {
	CNODE head, tail;
	mutex glock;
public:
	CSET() {
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~CSET() {

	}

	void Init()	{
		CNODE *ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}
	bool Add(int key) {
		CNODE *pred, *curr;
		head.lock();
		pred = &head; // 이 전 노드는 헤드로 한다.
		pred->next->lock();
		curr = pred->next; // 헤드의 넥스트, 테일을 커런트로 한다.
		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr->next->lock();
			curr = curr->next;
		}
		if (key == curr->key) {
			curr->unlock();
			pred->unlock();
			return false;
		}
		else {
			CNODE* node = new CNODE(key);
			node->next = curr;
			pred->next = node;
			curr->unlock();
			pred->unlock();
			return true;			
		}
	}
	bool Remove(int key) {
		CNODE *pred, *curr;
		head.lock();
		pred = &head;
		pred->next->lock();
		curr = pred->next;
		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr->next->lock();
			curr = curr->next;
		}
		if (key == curr->key) {
			pred->next = curr->next;
			delete curr;
			curr->unlock();
			pred->unlock();
			return true;
		}
		else {
			curr->unlock();
			pred->unlock();
			return false;
		}
	}

	bool Contains(int key) {
		CNODE *pred, *curr;
		head.lock();
		pred = &head;
		pred->next->lock();
		curr = pred->next;
		while (curr->key < key) {
			pred->unlock();
			pred = curr;
			curr->next->lock();
			curr = curr->next;

		}
		if (key == curr->key) {
			curr->unlock();
			pred->unlock();
			return true;
		}
		else {
			curr->unlock();
			pred->unlock();
			return false;
		}
	}
};
CSET cSet;
const auto NUM_TEST = 4000000;
const auto KEY_RANGE = 1000;

void List_ThreadFunc(int num_thread) {
	int key;


	for (auto i = 0; i < NUM_TEST / num_thread; ++i) {
		switch (rand() % 3) {
		case 0: 
			key = rand() % KEY_RANGE;
			cSet.Add(key);
			break;
		case 1:
			key = rand() % KEY_RANGE;
			cSet.Remove(key);
			break;
		case 2:
			key = rand() % KEY_RANGE;
			cSet.Contains(key);
			break;
		default:
			cout << "Error \n";
			exit(-1);
		}
	}

}
int main()
{
	std::vector<std::thread*> v_threadlist;
	v_threadlist.reserve(8);

	for (auto i = 1; i <= v_threadlist.capacity(); i *= 2) {
		auto start_time = std::chrono::high_resolution_clock::now();
		cSet.Init();
		for (auto j = 0; j < i; ++j) {
			v_threadlist.push_back(new std::thread{ List_ThreadFunc, i  });
		}
		for each (auto th in v_threadlist)
		{
			th->join();
			delete th;
		}
		auto end_time = std::chrono::high_resolution_clock::now() - start_time;
		v_threadlist.clear();
		cout << "thread 개수 : " << i << endl;
		cout << std::chrono::duration_cast<std::chrono::milliseconds>(end_time).count() << "msecons" << endl;
	}

	system("pause");

}