#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <windows.h>
using namespace std;


enum status { EMPTY, WAITING, BUSY };
const auto MAX_THREAD = 64;
const auto NUM_TEST = 25000000;
atomic_int c = 0;

_declspec(thread) static int range;

class Node {
public:
	int value;
	Node *next;
public:
	Node() { next = nullptr; value = 0; }
Node(int n) { value = n; next = nullptr; }
~Node() { }
};
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
class Exchange {
	const int delay = 100;
	int slot;
	int get_status() {
		return (slot >> 30) & 0x3;
	}
	void set_status(int status) {
		slot = (status << 30);
	}
	int get_value() {
		int val = (slot & 0x00ffffff);
		return (slot & 0x00ffffff);
	}
public:
	Exchange() { slot = 0; }
	bool CAS(int old_stat, int new_stat, int val) {
		int this_value = get_value();
		old_stat = (old_stat << 30) + this_value;
		new_stat = (new_stat << 30) + val;
		return atomic_compare_exchange_strong(
			reinterpret_cast<atomic_int *>(&slot),
			reinterpret_cast<int*>(&old_stat),
			new_stat);
	}
	int exchange(int value, bool *time_out) {
		while (true) {
			slot = 0;
			switch (get_status()) {
			case EMPTY:
				if (false == CAS(EMPTY, WAITING, value)) continue;
				int current, start;
				_asm RDTSC;
				_asm mov start, eax;
				do {
					if (BUSY == get_status()) {
						int ret = get_value();
						set_status(EMPTY);
						*time_out = false;
						return ret;
					}
					_asm RDTSC;
					_asm mov current, eax; // 다시 now();
				} while (current - start < delay);
				if (false == CAS(WAITING, EMPTY, 0)) {
					int ret = get_value();
					*time_out = false;
					return ret;
				}
				*time_out = true;
				return 0;
				break;
			case WAITING: 
				if (false == CAS(WAITING, BUSY, value)) continue;
				*time_out = false;
				return get_value();  
				break;
			case BUSY: continue; break;
			default:
				cout << "Undefined State Error\n";
				exit(-1);
			}
		}
	}
};
class Elimination {
	Exchange exchange_array[MAX_THREAD];
public:
	int visit(int value, int range, bool *time_out) {
		if (range >= MAX_THREAD) range = MAX_THREAD;
		int index = rand() % ( range );
		return exchange_array[index].exchange(value, time_out);
	}

};

class ELFStack {
	Node * volatile top;
	Elimination el;
public:
	ELFStack() {
		top = nullptr;
	}
	~ELFStack() {
		Init();
	}
	bool CAS(Node* volatile* pred, Node* old_value, Node* new_value) {
		return atomic_compare_exchange_strong(
			reinterpret_cast<atomic_int volatile*>(pred),
			reinterpret_cast<int*>(&old_value), reinterpret_cast<int>(new_value));
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
		node->next = top;
		return CAS(&top, old_top, node);
	}
	void Push(int key) {
		Node *node = new Node(key);
		while (false == try_Push(node)) {
			bool time_out;
			int ret = el.visit(key, range, &time_out);
			if (time_out) continue;
			if (0 == ret) {
				delete node;
				return;
			}
		}
	}
	Node* try_Pop() {
		Node *old_top = top;
		if (nullptr == old_top) {
			cout << "empty stack\n";
			return nullptr;
		}
		Node *next = old_top->next;
		if (true == CAS(&top, old_top, next))
		{
			return old_top;
		}
		else return nullptr;
	}
	int Pop() {
		while (true) {// try_pop을 호출하여 스택의 첫 노드를 제거하려고 시도 한다. 
			Node* return_node = try_Pop();
			if (return_node != nullptr) return return_node->value;
			bool time_out;
			int ret = el.visit(0, range, &time_out);
			if (time_out) continue;
			if (0 != ret) {
				//c++;
				return ret;
			}
		}

	}
};

ELFStack elfStack;

void test_thread_func(int thread_num) {
	range = thread_num;
	for (auto i = 0; i < NUM_TEST / thread_num; ++i) {
		if (i < ((NUM_TEST / thread_num) / 2) /2)
		{
			elfStack.Push(i);
			continue;
		}
		if ((rand() % 2)) {
			elfStack.Push(i);
		}
		else {
			elfStack.Pop();
		}
	}
}
int main() {
	vector<thread*> v_treads;
	for (auto i = 1; i <= MAX_THREAD; i *= 2) {
		v_treads.clear();
		elfStack.Init();
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
		//	cout << elfStack.Pop() << ":";
		//}cout << endl;
		//cout << " 소거 된 횟수 : " << c << endl;
		//c = 0;
	}
	while (true);
}