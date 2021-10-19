
#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

using namespace std;

class CNODE {
public:
	int key;
	mutex glock;
	bool marked;
	CNODE *next;


public:
	CNODE() {
		next = nullptr;
		marked = false;
	}
	CNODE(int num) {
		key = num;
		marked = false;
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
CNODE *free_list = nullptr;
mutex glock;
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
	void Init() {
		CNODE* ptr;
		while (head.next != &tail) {
			ptr = head.next;
			head.next = head.next->next;
			delete ptr;
		}
	}
	bool validate(CNODE *pred, CNODE *curr) {
		return ((!pred->marked) && (!curr->marked) && (pred->next == curr));
	}
	bool Add(int key) {
		CNODE* pred, *curr;
		while (1) {
			pred = &head;
			curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();
			curr->lock();
			if (!validate(pred, curr)) {
				pred->unlock();
				curr->unlock();
				continue;
			}
			if (key == curr->key) {
				pred->unlock();
				curr->unlock();
				return false;
			}
			else {
				CNODE *node = new CNODE(key);
				node->next = curr;
				pred->next = node;
				pred->unlock();
				curr->unlock();

				return true;
			}
		}
	}
	bool Remove(int key) {
		CNODE *pred, *curr;
		while (1) {
			pred = &head;
			curr = pred->next;
			while (curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			pred->lock();
			curr->lock();
			if (!validate(pred, curr)) {
				pred->unlock();
				curr->unlock();
				continue;
			}
			if (key == curr->key) {
				curr->marked = true;
				pred->next = curr->next;

				//delete list
				//glock.lock();
				//curr->next = free_list;
				//free_list = curr;
				//glock.unlock();

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
	}

	bool Contains(int key) {
		CNODE *curr = &head;
		while (curr->key < key)
			curr = curr->next;
		return ((curr->key == key) && (!curr->marked));
	}
};
CSET cSet;
const auto NUM_TEST = 40000000;
const auto KEY_RANGE = 100;
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
	v_threadlist.reserve(16);

	free_list = new CNODE(0x7fffffff);
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

		while (nullptr != free_list->next) {
			CNODE *ptr = free_list;
			free_list = free_list->next;
			delete ptr;
		}

		cout << "Thread 개수 : " << i;
		cout << " , Time : " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time).count() << "msecons" << endl;
	}
	system("pause");

}