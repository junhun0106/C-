/*
2016년 4월 11일 월요일
전공특강

동기화 구현

Steap.4 낙천적인 동기화(optimistic)

세밀한 동기화에 경우 Lock, Unlock이 빈번하여 리스트가 길어지는 경우
성능이 매우 떨어진다.


*/

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
public:
	CSET() {
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.next = &tail;
	}
	~CSET() {

	}
	bool validate(CNODE *pred, CNODE *curr) {
		CNODE *node = &head;
		while (node->key <= pred->key) {
			if (node == pred)
				 return (pred->next == curr);
			node = node->next;
		}
		return false;
	}
	void Init() {
		CNODE *ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}
	bool Add(int key) {
		while (1) {
			CNODE *pred, *curr;
			pred = &head; // 이 전 노드는 헤드로 한다.
			curr = pred->next; // 헤드의 넥스트, 테일을 커런트로 한다.
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();
			curr->lock();
			if (validate(pred, curr)) {
				if (key == curr->key) {
					pred->unlock();
					curr->unlock();
					return false;
				}
				else {
					CNODE* node = new CNODE(key);
					node->next = curr;
					pred->next = node;
					pred->unlock();
					curr->unlock();

					return true;
				}
			}
			else {
				pred->unlock();
				curr->unlock();
				continue;
			}
		}
		return false;
	}
	bool Remove(int key) {
		while (1) {
			CNODE *pred, *curr;
			pred = &head;
			curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();
			curr->lock();
			if (validate(pred, curr)) {
				if (key == curr->key) {
					pred->next = curr->next;
					pred->unlock();
					curr->unlock();
					return true;
				}
				else {
					pred->unlock();
					curr->unlock();
					return false;
				}
			}
			else {
				pred->unlock();
				curr->unlock();
				continue;
			}
		}
		return false;
	}

	bool Contains(int key) {
		while (1) {
			CNODE *pred, *curr;
			pred = &head;
			curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;

			}
			pred->lock();
			curr->lock();
			if (validate(pred, curr)) {
				if (key == curr->key) {
					pred->unlock();
					curr->unlock();
					return true;
				}
				else {
					pred->unlock();
					curr->unlock();
					return false;
				}
			}
			else {
				pred->unlock();
				curr->unlock();
				continue;
			}
		}
		return false;
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
			v_threadlist.push_back(new std::thread{ List_ThreadFunc, i });
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
