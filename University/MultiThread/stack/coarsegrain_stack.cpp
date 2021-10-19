#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>

using namespace std;

class Node {
public:
	int value;
	Node *next;
public:
	Node() { next = nullptr; value = 0; }
	Node(int n) { value = n; next = nullptr; }
	~Node() { }
};
class CStack {
	Node *top;
	mutex m_lock;
public:
	CStack() {
		top = nullptr;
	}
	~CStack(){ }

	void Init() {
		Node *ptr;
		while (top != nullptr) {
			ptr = top;
			top = top->next;
			delete ptr;
		}
	}

	void Push(int key) {
		m_lock.lock();
		Node *node = new Node(key);
		node->next = top;
		top = node;
		m_lock.unlock();
	}
	int Pop() {
		m_lock.lock();
		if (nullptr == top) {
			m_lock.unlock();
			cout << " empty stack \n";
			return 0;
		}
		int result = top->value;
		Node *temp = top;
		top = top->next;
		delete temp;
		m_lock.unlock();
		return result;
	}
};

const auto NUM_TEST = 25000000;
CStack cStack;

void test_thread_func(int thread_num) {
	for (auto i = 1; i < NUM_TEST / thread_num; ++i) {
		if ((rand() % 2) || i < 1000 / thread_num) {
			cStack.Push(i);
		}
		else {
			cStack.Pop();
		}
	}
}
int main() {
	vector<thread*> v_treads;
	for (auto i = 1; i <= 16; i *= 2) {
		v_treads.clear();
		cStack.Init();
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

		cout << "stack:";
		for (auto i = 0; i < 20; ++i) {
			cout << cStack.Pop() << ":";
		}cout << endl;
	}
	while (true);
}