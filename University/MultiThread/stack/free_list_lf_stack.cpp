#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

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
bool CAS(Node* volatile* pred, Node* old_value, Node* new_value) {
	return atomic_compare_exchange_strong(
		reinterpret_cast<atomic_int volatile*>(pred),
		reinterpret_cast<int*>(&old_value), reinterpret_cast<int>(new_value));
}
class Backoff {
	const int max_delay = 400000;
	int limit;
public:
	Backoff() {
		limit = 100;
	}
	void wait() {
		int delay;
		delay = rand() % limit;
		if (limit < max_delay) limit = 2 * limit;
		int start, current;
		_asm RDTSC;
		// ReaD, Time, Stamp, Counter
		// TSC 라는 64bit 레지스터가 있다. 컴ㅍ터를 딱 키면 0이 되고, 클럭 하나 돌아 갈 때 마다 카운터가 1씩 증가 한다.
		// chrono::high....:: now() 하는 것과 비슷한 행위
		_asm mov start, eax;
		do { // now() - start;
			_asm RDTSC;
			_asm mov current, eax; // 다시 now();

		} while (current - start < delay);
	}
};
class FreeList {
	Node *volatile first;
	Node *volatile second;
public:
	FreeList() { first = new Node(0); second = new Node(0); }
	~FreeList() {
		while (first != nullptr) {
			Node *temp = first;
			first = first->next;
			delete temp;
		}
		while (second != nullptr) {
			Node *temp = second;
			second = second->next;
			delete temp;
		}
	}
	void Exchange() { 
		Node* temp = first;
		first = second;
		second = temp;
	}

	Node* GetNode(int value) {
		Backoff backoff;
		while (true) {
			Node* head = first;
			Node *next = head->next;
			if (nullptr == next) return (new Node(value));
			if (CAS(&first, head, next))
			{
				head->value = value;
				return head;
			}
			backoff.wait();
		}
	}
	void FreeNode(Node *free_node) { 
		Backoff backoff;
		while (true) {
			Node* head = second;
			free_node->next = head;
			if (CAS(&second, head, free_node)) return;
			backoff.wait();
		}
	}
};

FreeList free_list;

class LFStack {
	Node * volatile top;
public:
	LFStack() {
		top = nullptr;
	}
	~LFStack() {
		Init();
	}
	void Init() {
		Node *ptr;
		while (top != nullptr) {
			ptr = top;
			top = top->next;
			delete ptr;
		}
	}
	bool try_Push(Node* node) {
		Node *old_top = top;
		node->next = old_top;
		return CAS(&top, old_top, node);
	}
	void Push(int key) {
		Node *node = free_list.GetNode(key);
		Backoff backoff;
		while (false == try_Push(node))
			backoff.wait();
	}
	Node* try_Pop() {
		Node *old_top = top;
		if (nullptr == old_top) {
			//cout << "empty stack\n";
			return nullptr;
		}
		Node *next = old_top->next;
		if (true == CAS(&top, old_top, next))
		{
			free_list.FreeNode(old_top);
			return old_top;
		}
		else return nullptr;
	}
	int Pop() {
		Backoff backoff;
		while (true) {// try_pop을 호출하여 스택의 첫 노드를 제거하려고 시도 한다. 
			Node* return_node = try_Pop();
			if (return_node != nullptr)
			{
				//free_list.FreeNode(return_node);
				return return_node->value;
			}
			backoff.wait(); // 대기
		}
	}
};

const auto NUM_TEST = 25000000;
LFStack lfStack;

void test_thread_func(int thread_num) {
	for (auto i = 0; i < NUM_TEST / thread_num; ++i) {
		if ((rand() % 2) || i < 10000 / thread_num) {
			lfStack.Push(i);
		}
		else {
			lfStack.Pop();
		}
	}
}
int main() {
	vector<thread*> v_treads;
	for (auto i = 1; i <= 16; i *= 2) {
		v_treads.clear();
		lfStack.Init();
		free_list.Exchange();
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
		//cout << "stack:";
		//for (auto i = 0; i < 20; ++i) {
		//	cout << lfStack.Pop() << ":";
		//}cout << endl;
	}
	while (true);
}