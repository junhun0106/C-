/*
stack
후입선출(LIFO) 구조
push, pop

무제한 무잠금 스택
무잠금 스택은 연결 리스트로 구성되고 top 필드가 첫 노드를 가리킨다.
- 만약 스택이 비어 있을 경우는 NULL(0은 넣지 못하게 한다. NULL과 0의 혼돈을 막기 위해서)
- NULL 값을 스택에 추가하는 것은 고려하지 않는다.
- push, pop 메서드 둘다 CAS를 이용한다
	- pop은 완전 메서드. 비어 있을 경우에도 return을 해야 하는데, null(empty)일 경우 0을 retrun하고 끝내도록 만드는게 구현하는데 편하기 쉬우니까.
	- 
- 경쟁이 심할 경우 Backoff를 시도한다

＃ BackOff ??
- 경쟁이 심할 경우 CAS를 계속 시도하는 것은 넌체 시스템에 악영향을 줌.
	- queue의 경우 head와 tail이 cas에 걸릴 경우 최대 병렬성이 2 밖에 안된다. stack은 top밖에 없으므로 경쟁이 더 심해 진다.
	- 캐시 핑퐁으로 다른 일을 하고 있는 스레드마저도 느려진다.
- 따라서 실패했을 경우 일정 기간 실행을 중간 했다가 다시 실행하는 것이 좋음
	- nano 단위로 쉬어야 한다. 1나노 당 명령어가 30개씩이나 움직이다. 약 10나노 부터 쉬게 해보자.
- 실행 중단 기간
	- back off를 하는 객체들은 서로 다른 중단 기간을 가져야 한다. 
		- 시간이 고정되어 있으면, 같이 쉬러 들어간 스레드들이 같이 일어나 또 다시 경쟁을 한다.
		- 서로 다른 시간으로 재워야 한다. CAS가 몰리면 몰릴 수록 CAS는 Aatomic하게 실행 되므로 충돌 한 스레드 수 만큼 시간이 늘어 난다!!
	- 충돌이 많으면 오래 기다리고, 충돌이 적으면 적게 기다리게 해야 한다.
		- 몇 개의 스레드가 충돌 했는지 검사 하는 것도 오버헤드!! 
	- 따라서, 처음에는 짧게 계속 실패하면 계속 길게 성공 할 때 까지


결론
 - 이 구현은 락프리이다.
 - 각 push나 pop 매서드는 스택의 탑을 수정하는 무한히 많은

문제
 - delete를 안해서 ABA 문제가 안생긴다 -> 넣으면 생김!! 
 - ABA가 QUEUE보다 훨씬 더 높은 확률로 생긴다.
 - 64비트 stamp를 사용해서 해결할 필요가 있다.
 - 하지만 64비트 operation이 불가능한 경우는?

free_list
 - 64비트에 의존하지 않는다.
 - 완벽하고 깔끔한 방법은 아니다.
 - 즉시 재사용이 아닌 단계별 재사용으로 ABA문제 해결

*/

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
	~Node(){ }
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
class LFStack {
	Node * volatile top;
public:
	LFStack() {
		top = nullptr;
	}
	~LFStack() {
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
		node->next = old_top;
		return CAS(&top, old_top, node);
	}
	void Push(int key) {
		Node *node = new Node(key);
		Backoff backoff;
		while (false == try_Push(node))
			backoff.wait();
	}
	Node* try_Pop() {
		Node *old_top = top;
		if (nullptr == old_top) {
			cout << "empty stack\n";
			exit(-1);
		}
		Node *next = old_top->next;
		if (true == CAS(&top, old_top, next)) return old_top;
		else return nullptr;
	}
	int Pop() {
		Backoff backoff;
		while (true) {// try_pop을 호출하여 스택의 첫 노드를 제거하려고 시도 한다. 
			Node* return_node = try_Pop();
			if (return_node != nullptr) return return_node->value;
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
	for (auto i = 1; i <= 64; i *= 2) {
		v_treads.clear();

		lfStack.Init();

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