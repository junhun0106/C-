/*
2016년 4월 14일 목요일
전공특강

coares 병렬성 X
fine Over head 많음
optimistic fine에 비해 오버헤드가 적지만, 오버헤드 존재, memory leak 존재.

게으른 동기화(Lazy acynchronos)
낙천적 동기화는 lock의 회수를 비약적으로 감소, but list를 2번 순회해야 한다.
validate가 노드를 처음부터 순회하지 않게 수행하게 하자.
pred와 curr의 잠금은 여전히 필요하다.

자료구조의 검색 메소드는 add, delete에 비해 자주 호출되는 함수 이므로, wait-free로 만들 수 있으면 좋겠다.

validate 조건.
pred와 curr가 리스트에 존재해야 한다.
pred와 curr가 연결되어 있어야 한다. ( 사이에 다른 노드가 존재 X )
pred와 curr가 lock이 되어 있어야 한다.

각 노드에 marked 필드(deleted)를 추가하여 그 노드가 집합에서 제거 되어 있는지 표시 한다. true면 제거되었다는 표시.
marking을 실제 제거 보다 바느시 "먼저" 수행한다. -> 노드가 사라지는 경우는 remove에서만 일어난다. remove되기 전에 마킹을 하고 없앤다.

memory leak을 방지 해야 한다.
free_list 사용 -> 메모리를 재사용 할 수 없고, 스레드를 닫고 메모리를 delete를 해줘야 한다.
제대로 된 멀티스레드는 스레드를 종료하고 다시 시작하는 일을 하지 않고, 따라서 delete를 할 수 없으니
똑같이 memory leak이 발생 한다.

자바 c#과 비슷한 shared_ptr를 사용해보자.
디스어셈블리에서 했을대 shared_ptr은 재귀를 사용 한다.
재귀는 stack을 매우 많이 사용 한다.
멀티스레드에서는 스택이 하나 더 필요하고, 남의 스택을 차지하는 순간 뻗어버리는 것 이다.(스택 쫑이 나는 순간)
이걸 막기 위해서는 스택의 크기를 늘려야 하는데, c++11은 제공하지 않는다. -> CreateThread 윈도우 함수로 할 수 있다.

가끔 사용하는 몇 개 담지 않는 자료구조는 shared_ptr를 사용해도 상관 없으나, 그렇지 않은 것은 사용하지 않는다.

메모리 관리를 효율적으로 관리하기 위해서는 커스텀 shared_ptr를 만들거나, free_list를 제대로 만들어야 한다.
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
	bool marked;
	//CNODE *next;
	shared_ptr<CNODE> next;

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
	shared_ptr<CNODE> head, tail;
public:
	CSET() {
		head = make_shared<CNODE>(0x80000000);
		tail = make_shared<CNODE>(0x7FFFFFFF);
		head->next = tail;
	}
	~CSET() {
	}
	void Init() {
		shared_ptr<CNODE> ptr;
		head->next = tail;
		// shared_ptr이기 때문에 head가 tail을 가리키면, 그 사이에 있던 날릴 때, 하나 하나 delete를 해주어야 했던 것을
		// 대신 해준다.
		//while (head->next != tail) {
		//	ptr = head->next;
		//	head->next = head->next->next;
		//}
	}
	bool validate(shared_ptr<CNODE> pred, shared_ptr<CNODE> curr) {
		return ((!pred->marked) && (!curr->marked) && (pred->next == curr));
	}
	bool Add(int key) {
		shared_ptr<CNODE> pred, curr;
		while (1) {
			pred = head; // 이 전 노드는 헤드로 한다.
			curr = pred->next; // 헤드의 넥스트, 테일을 커런트로 한다.
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
				shared_ptr<CNODE> node = make_shared<CNODE>(key);
				node->next = curr;
				pred->next = node;
				pred->unlock();
				curr->unlock();

				return true;
			}
		}
	}
	bool Remove(int key) {
		shared_ptr<CNODE> pred, curr;
		while (1) {
			pred = head;
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

				// shared_ptr이므로 free_list는 없어도 된다.
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
		shared_ptr<CNODE>curr = head;
		while (curr->key < key)
			curr = curr->next;
		return ((curr->key == key) && (!curr->marked));
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

	cout << "shared_ptr \n";

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

		//while (nullptr != free_list->next) {
		//	CNODE *ptr = free_list;
		//	free_list = free_list->next;
		//	delete ptr;
		//}

		cout << "thread 개수 : " << i << endl;
		cout << std::chrono::duration_cast<std::chrono::milliseconds>(end_time).count() << "msecons" << endl;
	}

	system("pause");

}
