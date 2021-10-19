/*
2016년 4월 18일 월요일
전공특강

non-blocking programming

쓰레드간의 데이터 공유 및 동기화는 안전한 "Lock_free" 자료구조를 통해서 이루어 진다!!
여러 개의 쓰레드에서 동시에 호출 됐을 때에도 정해진 단위 시간에 적어도 한 개의 호출이 완료되는 알고리즘!!
멀티쓰레드에서 동시에 호출해도 정확한 결과!! -> STL X 멀티쓰레드에서 죽음.
Non-Blocking 다른 쓰레드가 어떤 상태에 있건 상관 없이 호출이 완료.
호출이 충돌하였을 경우 적어도 하나의 승자가 있어서, 승자는 Delay X

비멈춤 동기화
Lock_free()
ADD(), Remove()는 lock_free.
wait_free로 구현하면 프로그램이 지저분해짐.
Contains()는 Wait-free로 구현.

CAS의 한계
- 한번에 하나의 변수 밖에 바꾸지 못한다. -> 여러 곳을 동시에 감시 할 수가 없다.
따라서, 감시를 해야 하는 개수를 줄어야 한다.(예방책을 사용 한다.) -> marking이 필요하다.
Remove시 포인터도 바꿔줘야하고, marking도 같이 확인해야 한다 -> 어떻게 CAS로 동시에 2개 확일 할 까??
마찬가지로 pred, curr도 다른 쓰레드가 날린건지 아닌지 알아야 한다. -> 여러 개 CAS 필요??

CAS의 개수를 어떻게 추리는가 ?? -> next와 marked를 하나로 합쳐보자!! 멀티 CAS 구현!!
CAS(oldmark, mark, oldnext, next) -> CAS 하나에서 한 개 밖에 안되는데 어떻게 ?! -> next 필드 중에 남는 부분을 marked를 이용해보자.
next필드는 32비트. new를 할 때는 4의 배수로 할당 한다. CDB 때문에... 맨 뒤 2개는 00으로 채워져 있다.(아무런 의미가 없는 비트이다.) 이것을 사용하면 된다!!

non-blocking 자료구조
1. lock 없음 -> 한 번에 atomic operation으로 모든 것을 끝내야 함!!
2. atomic operation - read, write, CAS.
CAS - 한 번에 메모리 하나만 수정 가능. 변경해야 할 곳이 여러 곳일 때는?
3. 대책 : 가능한 한 모든 변경점을 한 곳으로 모으자.

*/

#include <iostream>
#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
#include <concurrent_queue.h>

using namespace std;
using namespace concurrency;



class CNODE {
public:
	int key;
	int next_marked; // 주소 + 마크

public:
	CNODE() {
		next_marked = 0;
	}
	CNODE(int num) {
		key = num;
		next_marked = 0; // NULL + FALSE
	}
	~CNODE() {
	}

	void SetKey(int n) { key = n; }
	void SetNext(CNODE *other) {
		next_marked = reinterpret_cast<int>(other) & 0xfffffffe;
	}

	CNODE* GetNextAddr(){ 
		return reinterpret_cast<CNODE*>(next_marked & 0xfffffffe);
	}
	CNODE* GetNextMarkable(bool *marked) {
		*marked = ((next_marked & 1) == 1);
		return reinterpret_cast<CNODE*>(next_marked & 0xfffffffe);
	}
	bool CAS(int old_v, int new_v) { return atomic_compare_exchange_strong(reinterpret_cast<atomic_int *>(&next_marked), &(old_v), new_v); }
	bool CAS(CNODE *old_node, CNODE* new_node, bool old_mark, bool new_mark) {
		int old_v = reinterpret_cast<int>(old_node);
		if (true == old_mark) old_v = old_v | 1;
		else old_v = old_v & 0xFFFFFFFE;

		int new_value = reinterpret_cast<int>(new_node);
		if (new_mark) new_value = new_value | 1;
		else new_value = new_value & 0xfffffffe;

		return CAS(old_v, new_value);
	}
	bool TryMarked(CNODE *next) {
		int old_value = reinterpret_cast<int>(next) & 0xfffffffe;
		int new_value = old_value | 1;
		return CAS(old_value, new_value);
	}
};

class CFREE_LIST {
	CNODE* free_list;
	CNODE* reuse_list;

	CNODE sentinel_1, sentinel_2;

public:
	CFREE_LIST() {
		sentinel_1.next_marked = 0;
		sentinel_1.key = 0x70000000;
		free_list = &sentinel_1;

		sentinel_2.next_marked = 0;
		sentinel_2.key = 0x70000000;
		reuse_list = &sentinel_2;
	}
	~CFREE_LIST() {

	}
	void clear() {
		CNODE* temp = free_list;
		free_list = reuse_list;
		reuse_list = temp;
	}
	CNODE* GetNode(int key) {
		if (reuse_list->GetNextAddr() == nullptr) return (new CNODE(key));
		while (true) {
			CNODE *temp = reuse_list;
			CNODE *next = temp->GetNextAddr();
			CNODE *succ = next->GetNextAddr();

			// 얻거나 집어넣을때 lock_free로 구현해야 한다.
			if(temp->CAS(next, succ, true, false)) // free_list에 있는 것은 마킹 되어진 노드이기 때문에 재사용하기 위해서는 true를 false로 바꿔주어야 한다.
			return temp;
		}

	}
};

concurrency::concurrent_queue<CNODE*> free_queue;
CNODE *free_list = nullptr;
mutex glock;

class CSET {
	CNODE head, tail;
public:
	CSET() {
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		head.SetNext(&tail);
	}
	~CSET() {
	
	}
	void Init() {
		CNODE* ptr;
		while (head.GetNextAddr() != &tail) {
			ptr = head.GetNextAddr();
			head.SetNext(head.GetNextAddr()->GetNextAddr());
			delete ptr;
		}
	}

	/*bool validate(CNODE *pred, CNODE *curr) {
	// 마크가 되있는 것이 있을 수 없다. 삽입, 삭제 전에 모두 다 제거하기 때문에
	// CAS가 있기 때문에 메모리 비교도 할 필요가 없다.
		return ((!pred->marked) && (!curr->marked) && (pred->next == curr));
	}*/

	void Find(CNODE **pred, CNODE **curr, int key) {		
		CNODE *succ;
		fail_retry:
		*pred = &head;
		*curr = (*pred)->GetNextAddr();
		while (true) {
			bool curr_is_removed;
			succ = (*curr)->GetNextMarkable(&curr_is_removed);
			while (curr_is_removed) {
				if(false == (*pred)->CAS((*curr), succ, false, false)) goto fail_retry;
				(*curr) = succ;
				succ = (*curr)->GetNextMarkable(&curr_is_removed);
			}
			if ((*curr)->key >= key) return;
			// foward
			(*pred) = (*curr);
			(*curr) = succ;
		}

	}
	bool Add(int key) {
		while (1) {
			CNODE* pred, *curr;
			Find(&pred, &curr, key);
			if (curr->key == key) return false;
			else {
				CNODE *node = new CNODE(key);
				//auto free_node = free_queue.unsafe_begin();
				//if (free_node == free_queue.unsafe_end())
				//node = new CNODE(key); // free_queue.GetNode(key);
				//else {
				//	(*free_node)->key = key;
				//	(*free_node)->next_marked = 0;
				//	node = (*free_node);
				//	if (!free_queue.try_pop((*free_node))) {
				//		delete node;
				//		continue;
				//	}
				//}
				node->SetNext(curr);
				if (pred->CAS(curr, node, false, false))
					return true;
				else { delete node; continue; }
			}
		}
	}
	bool Remove(int key) {
		CNODE *pred, *curr;
		bool snip;
		while (1) {
			Find(&pred, &curr, key);
			if (curr->key != key) { return false; }
			else {
				CNODE *succ = curr->GetNextAddr();
				snip = curr->TryMarked(succ);
				if (!snip) continue;
				pred->CAS(curr, succ, false, false); // find에서 마킹 된 노드를 날려버리므로, 성공하든 말든 배째.
				
				//memory leak 해결 방안?!
				//Lazy와 같다. free_list로 재사용 혹은 shared_ptr
				//free_list는 런타임시 delete timing을 잡기 어렵다.
				//shared_ptr는 c++11에서 제공된 것은 스택 오버플로우의 문제와 속도에 문제가 심하다.

				//free_list의 개선이 필요하다!! 
				//기존의 lazy에서 사용 했던 방식은 lock이 들어가 있다. -> blocking에서 발생하는 문제가 생긴다!!
				//non_blocking free_list가 필요하다!! -> lock_free로 구현해야 한다!! -> LF_QUEUE 필요!!!
				//또 한 free_list에서 delete보다는 메모리를 재사용을 하는 것이 성능 향상에 좋다!! -> 어떻게 할 것 인가?!

				//free_queue.push(curr);

				return true;
			}
		}
	}
	bool Contains(int key) {
		bool curr_is_removed;
		CNODE *curr = &head;
		while (curr->key < key) curr = curr->GetNextMarkable(&curr_is_removed);
		return ((curr->key == key) && (!curr_is_removed));
	}

	CNODE* begin() { return &head; }
	CNODE* end() { return &tail; }
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
	v_threadlist.reserve(16);

	free_list = new CNODE(0x7fffffff);
	for (auto i = 1; i <= v_threadlist.capacity(); i *= 2) {
		auto start_time = std::chrono::high_resolution_clock::now();
		cSet.Init();
		//free_queue.clear();
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

		cout << "Thread 개수 : " << i;
		cout << " , Time : " <<std::chrono::duration_cast<std::chrono::milliseconds>(end_time).count() << "msecons" << endl;

		CNODE *ptr = cSet.begin()->GetNextAddr();
		for (auto i = 0; i < 10; ++i) {
			cout<< ptr->key << " ";
			ptr = ptr->GetNextAddr();
		}
		cout << endl;
	}
	system("pause");
}