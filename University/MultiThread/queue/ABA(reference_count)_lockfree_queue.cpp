/*
2016년 5월 19일 월요일

1. 헤드에 따라 노드들이 쭉 연결
테일은 가장 마지막에.

인큐하면 테일 뒤에
디큐하면 헤드에서.

------------------------------------

프로그램을 여러 개 실행 시키니 죽는다. 왜 죽느냐?!
ABA 때문에!!

ABA : 
스레드 A가 A를 deq하려고 cas하려는 순간 스레드 스케줄링에서 제외 됨. 그럴 수 있음.
그 사이 스레드 B가 신나게 deq를 하고, delete하고 열심히 지운 후에 enq도 열심히 했다.
기존에 있던 노드 메모리를 delete했기 때문에 빈 메모리를 재사용하다보니, 우연찮게 a의 메모리가 다시 맨 앞으로 붙었다.
그때 스레드 A가 일어나 cas를 시도 한다. a의 메모리가 head에 존재 하기 때문에 cas가 성공 한다. 그 전 b 메모리는 어딨는지 모르고,
재사용 瑛?수도 있다. 하지만 cas는 성공한다. 
queue가 깨져버렸다.

결론, delete한 메모리를 다시 사용해 다시 맨 앞으로 돌아았을 경우 cas를 시도 하던 스레드가 다시 돌아왔을 때 문제가 생길 수 있다!!
ABA!!

해결책:
1. 포인터를 포인터 + 스탬프로 확장. 포인터 값을 변경 할 때 마다 스탬프 값을 변경 한다.
- 카운터를 사용 한다. 초기에는 0이다. 헤드의 주소 값이 바뀌면 카운터를 증가.
a가 다시 돌아 와서 ABA 문제가 생길 경우 레퍼런스 카운터가 다를 경우 다른 a라는 것을 알 수 있다.
- cas는 한 번에 할 수 없다. 비트에 우겨 넣어 볼까? 32bit는 비트 수가 모자르다. 
- 64bit로 해보자. 64bit는 ABA의 확률이 확 줄어 든다. (거의 있을 수 없다.)


2. LL(load linked), SC(store conditional)명령의 사용
LL: 메모리를 읽는 명령어
SC: 메로리를 쓰는 명령어, next에 새로운 값을 쓰는데, 누군가 next를 건들게 되면 쓰지 말고 아무도 건들이지 않았을 때만 써라.
한 번에 읽고 쓰는 것이 아니라, 읽고 쓰는 것을 나눠놨기 때문에 읽고 나서 스레드가 잠들더라도 SC자체가 누군가 건들면 바로 실패하므로,
ABA문제가 생기지 않는다.
ARM,알파 등에 cpu에서 자주 사용 된다. -> 핸드폰에서 자주 사용 된다.
인텔 cpu에는 제공 안되지롱 ㅜ ㅜ
CAS보다 우월하다. 좋다. but wait_free 불가능.
- 단점 : 하드웨어 도움이 필요함

3. 레퍼런스 카운터, shared_ptr를 사용 한다.
- first가 첫 번째 노드를 가리키고 있는 동안에는 그 노드는 free_list에 들어 갈 수 없다.
- 단점 : 속도 메모리 용량

*/
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <Windows.h>

using namespace std;

atomic<int> testcount = 0;

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
	volatile long long  l_head;
	volatile long long  l_tail;
public:
	QUEUE() {
		long l = 0;
		l_tail = l_head = (l << 32) + reinterpret_cast<volatile long long>(new Node(0));
	}
	~QUEUE() {
		Node* ptr = reinterpret_cast<Node*>(l_head);
		delete ptr;
	}
	void Init() {
		Node *ptr;
		while (GetNode(l_head) != GetNode(l_tail)) {
			ptr = GetNode(l_head);
			l_head = reinterpret_cast<volatile long long>(ptr->next);
			delete ptr;
		}
		l_head = 0x00000000ffffffff & l_head;
		l_tail = 0x00000000ffffffff & l_tail;

	}
	bool CAS(Node* volatile* pred, Node* old_value, Node* new_value) {
		return atomic_compare_exchange_strong(
			reinterpret_cast<atomic_int volatile*>(pred),
			reinterpret_cast<int*>(&old_value), reinterpret_cast<int>(new_value));
	}
	bool Interlocked_CAS(volatile long long* pred, long long old_value, long long new_value) {
		long long temp = InterlockedCompareExchange64(pred, new_value, old_value);
		return (temp == old_value);
	}
	//bool Interlocked_CAS(Node* volatile* pred, Node *old_node, Node* new_node, int old_count, int new_count) {
	//	volatile long long* mem = reinterpret_cast<volatile long long*>(pred);
	//	long long old_value = old_count;
	//	old_value = (old_value << 32) + reinterpret_cast<long long>(old_node);
	//	long long new_value = new_count;
	//	new_value = (new_value << 32) + reinterpret_cast<long long>(new_node);
	//	return Interlocked_CAS(mem, old_value, new_value);
	//}
	bool Interlocked_CAS(volatile long long* pred, Node *old_node, Node* new_node, int old_count, int new_count) {
		long long old_value = old_count;
		old_value = (old_value << 32) + reinterpret_cast<long long>(old_node);
		long long new_value = new_count;
		new_value = (new_value << 32) + reinterpret_cast<long long>(new_node);
		return Interlocked_CAS(pred, old_value, new_value);

	}
	Node* GetNode(volatile long long node) {
		return reinterpret_cast<Node* volatile>((node));
	}
	Node* volatile GetandStamp(volatile long long node, int *stamp) {
		*stamp = (node >> 32);
		return reinterpret_cast<Node* volatile>(node);
	}
	Node* GetNext(long long node) {
		Node *n = reinterpret_cast<Node* volatile>(node);
		n = n->next;
		return n;
	}
	void enq(int x) {
		Node *e = new Node(x);
		int stamp;
		while (true) {
			Node *last = GetandStamp(l_tail ,&stamp);
			Node *next = last->next;

			if (last != GetNode(l_tail)) continue;
			if (NULL == next) {
				if (CAS(&(last->next), NULL, e)) {
					//if (CAS(&tail, last, e))
					//l_tail = (0 << 32) + reinterpret_cast<volatile long long>(tail);
					Interlocked_CAS(&l_tail, last, e, stamp, stamp+1);
					return;
				}
			}
			else {
				//if (CAS(&tail, last, next))
				//l_tail = (0 << 32) + reinterpret_cast<volatile long long>(tail);
				Interlocked_CAS(&l_tail, last, next, stamp, stamp + 1); // tail이 마지막노드가 아니라, 다른 노드를 가르키고 있을때 tail 전진.
			}
		}
	}
	void empty_error() {
		cout << "empty_error" << endl;
	}
	int deq() {
		int last_count;
		int first_count;
		while (true) {
			Node *first = GetandStamp(l_head, &first_count);
			Node *last = GetandStamp(l_tail, &last_count);
			Node *next = first->next;

			if (first == last) { // tail이 미처 전진하지 못하는 경우, 꼬일 수 있으니 예방책.
				if (next == NULL) {
					empty_error();
					return 0;
				}
				Interlocked_CAS(&l_tail, last, next, last_count, last_count + 1);
				continue;// tail 전진
			}
			else {
				if (Interlocked_CAS(&l_head, first, next, first_count, first_count + 1))
				{
					int value = first->key;
					delete first;
					return value;
				}
			}
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