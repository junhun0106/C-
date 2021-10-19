#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

using namespace std;

class Node {
public:
	int key;
	Node* next;
public:
	Node() {
		next = nullptr;
	}
	Node(int x) {
		key = x;
		next = nullptr;
	}
};
class QUEUE {

	Node* volatile head;
	Node* volatile tail;

public:
	QUEUE() {
		head = new Node(0);
		tail = head;
	}
	~QUEUE() {
		delete head;
	}
	void Init() {
		Node *ptr;
		while (head != tail) {
			ptr = head;
			head = head->next;
			delete ptr;
		}
	}
	bool CAS(Node* volatile* pred, Node* old_value, Node* new_value) {
		return atomic_compare_exchange_strong(
			reinterpret_cast<atomic_int volatile*>(pred),
			reinterpret_cast<int*>(&old_value), reinterpret_cast<int>(new_value));

	}
	void enq(int x) {
		Node *e = new Node(x);
		while (true) {
			Node *last = tail;
			Node *next = last->next;
			if (last != tail) continue;
			if (NULL == next) {
				if (CAS(&(last->next), NULL, e)) {
					CAS(&tail, last, e);
					return;
				}
			} else CAS(&tail, last, next); // tail이 마지막노드가 아니라, 다른 노드를 가르키고 있을때 tail 전진.
		}
	}
	void empty_error() {
		cout << "empty_error" << endl;
		exit(-1);
	}
	int deq(){
		while (true) {
			Node *first = head;
			Node *last = tail;
			Node *next = first->next;
			if (first != head) continue;
			if (first == last){ // tail이 미처 전진하지 못하는 경우, 꼬일 수 있으니 예방책.
				if (next == NULL) {
					empty_error();
					return 0;
				}
				CAS(&tail, last, next); // tail 전진.
				continue;
			}
			int value = next->key;
			if (false == CAS(&head, first, next)) continue;
			delete first;
			return value;
		}
	}
};

const auto NUM_TEST = 10000000;
QUEUE lfQueue;

void test_thread_func(int thread_num)
{
	for (auto i = 0; i < NUM_TEST / thread_num; ++i) {
		if ((rand() % 2) || i < 1000 / thread_num) {
			lfQueue.enq(i);
		}
		else {
			lfQueue.deq();
		}
	}
}
int main() {
	vector<thread*> v_treads;
	for (auto i = 1; i <= 16; i *= 2) {
		v_treads.clear();

		lfQueue.Init();

		auto start = chrono::high_resolution_clock::now();
		for (auto j = 0; j < i; ++j)
			v_treads.push_back(new thread{ test_thread_func , i });
		for (auto th : v_treads) {
			th->join();
			delete th;
		}
		auto end = chrono::high_resolution_clock::now() - start;
		cout << i << " thread ";
		cout << " Result = " << chrono::duration_cast<chrono::milliseconds>(end).count()
			<< " ms " << endl;

		cout << "IN_QUEUE : ";
		for (auto i = 0; i < 10; ++i) {
			cout << lfQueue.deq() << " : ";
		}
		cout << endl;
	}
	while (true);
}