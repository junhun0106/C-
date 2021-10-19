/*
검색 자료구조
set  { 검색 중요 중복 허용 X } = set, map
pool { 중복허용 검색 X }

게임 서버에 player를 예를 들면 로그인, 로그아웃때만 insert, delete한다. 즉, 넣고 빼는 비율보다 검색하는 비율이 높다.
우리는 lf linked list를 배웠지만, 대게의 경우 리스트는 잘 쓰지 않는다. ( 검색이 느리기 때문 )
대부분은 tree 형식을 사용하는데 검색의 시간을 log(n)으로 줄이기 때문!!

트리는 재균형이 필요하다!! (heap, b+ redblack, avl과 같이)

락프리로 재균형을 어떻게 해야 할까 ???

skip list - 최신 자료구조

재균형 작업이 필요 없고, 워스트 케이스가 거의 나오지 않는다.(O(n)이 나올 확률이 무시 할 수 만큼 잘 나오지 않는다!!)
※재균형 작업을 CAS 한 방에 또 실패하더라도 뒤수습하는건 가능 하지만, 오버헤드와 병목, 경쟁상태를 백프로 유발하기 때문에 성능이 썩 좋지 않다!!

랜덤 자료구조!!

노드마다 랜덤한 길이의 포인터 배열을 갖고 있음.
같은 레벨의 포인터끼리 연결 됨.

링크드 리스트처럼 맨 루트는 넥스트들을 갖고 있지만
각 노드들의 높이가 랜덤으로 있고 높이마다 넥스트를 여러 개 갖고 있다.
크기가 높은 것 일수록 존재 확률이 적다.
맨 바닥 포인터는 바로 다음 것을 가르킨다.
두번 째 포인터는 길이가 2이상인 것만 가리킨다.(1인 것은 가리키지 않는다.) 
마찬가지로 n 번째 포인터는 높이가 n이상인 것만 가리킨다.
넥스트 포인터의 높이가 랜덤이고 그 포인터는 자기보다 같거나 큰 높이의 노드만 가르킨다.

이렇게 하면 검색을 log(n)에 할 수 있다??

검색의 구현 find()

포인터를 거꾸로 검사한다. head에서 제일 높은 곳에 있는 포인터를 본다. (센티널 노드는 제일 높은 높이를 갖고 있다 항상) = 3으로 가정
pred[3]이 가리키고 있는 것이 succ[]이다. 12를 찾는 다고 가정 했을 때, succ와 비교 한다.
이 succ를 기준으로 왼쪽에 있다는 결론을 얻을 수 있다.
pred[2]로 내려서 다시 succ[2]를 본다. 자신보다 큰지 아닌지를 판단하고 자기보다 크다면,
succ보다 크다면 succ -> pred가 대고, 그 높이에 있는 노드를 succ로 한다. 
succ보다 작다면 pred를 한 칸 내려 찾는다.

스킵리스트 속성은 항상 만족해야 한다. 높은 층의 리스트는 언제나 낮은 층의 리스트에 속한다.
보다 높은 층의 리스트는 낮은 층의 리스트에 대한 지름길이다. 계층의 각 연결은 바로 아래 계층의 2개의 노드를 건너 뛴다.

add의 추가 구현
리스트 add와 비슷.
단 pred와 curr의 배열들이 필요 한다.
같은 높이의 있는 것들을 넥스트로 해줘야 한다.

*/

// 성긴 동기화 순차 스킵 리스트

#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>
#include <Windows.h>

using namespace std;

const auto MAX_LEVEL = 10;
const auto MAX_THREAD = 64;
const auto NUM_TEST = 4000000;
const auto KEY_RANGE = 1000;

void random_number(int *height) {

	default_random_engine bdre;
	bernoulli_distribution bd(0.8);

	default_random_engine dre;
	if (bd(bdre))
	{
		uniform_int_distribution<int> uid(0, 6);
		*height = uid(dre);
	}
	else
	{
		uniform_int_distribution<int> uid(7, 9);
		*height = uid(dre);
	}

}
class Node {
public:
	int key;
	Node *next[MAX_LEVEL];
	int top_level;
	Node() {
		for (auto i = 0; i < MAX_LEVEL; ++i) next[i] = nullptr;
		top_level = MAX_LEVEL - 1; // 헷갈리므로 index와 height 맞추기.
	} 
	Node(int k){
		key = k;
		for (auto i = 0; i < MAX_LEVEL; ++i) next[i] = nullptr;
		top_level = MAX_LEVEL - 1;
	}
	Node(int k, int height) { // 높이는 1~10까지 1이면 1개짜리. 0이 들어오면 인덱스 깨짐.
		key = k;
		for (auto i = 0; i < MAX_LEVEL; ++i) next[i] = nullptr;
		top_level = height;
	}
	~Node() {

	}
};

class CSKIP_LIST {
	Node head, tail;
	mutex g_lock;
public:
	CSKIP_LIST(){
		head.key = 0x80000000;
		tail.key = 0x7FFFFFFF;
		for (auto i = 0; i < MAX_LEVEL ; ++i)
			head.next[i] = &tail;
	}
	~CSKIP_LIST(){
		Init();
	}
	void Init() {
		Node *ptr;
		while (head.next[0] != &tail) { // 바닥 포인터는 다 연결 되어 있기 때문에 바닥만 보면 된다.
			ptr = head.next[0];
			head.next[0] = head.next[0]->next[0];
			delete ptr;
		}
		for (auto i = 1; i < MAX_LEVEL; ++i) head.next[i] = &tail;
	}

	typedef Node *Nexts[MAX_LEVEL];

	//Node* (*pred)[MAX_LEVEL]
	//Node* (*curr)[MAX_LEVEL]

	void Find(int key, Nexts* pred, Nexts* curr)  {
		for (auto level = MAX_LEVEL - 1; level >= 0; --level) {
			if((MAX_LEVEL - 1) == level) (*pred)[level] = &head;
			else (*pred)[level] =(*pred)[level + 1];

			(*curr)[level] = (*pred)[level]->next[level];
			
			while ((*curr)[level]->key < key) {			
				(*pred)[level] = (*curr)[level];
				(*curr)[level] = (*curr)[level]->next[level];
			}
		}
	}
	
	bool Add(int key) {
		Nexts pred, curr;
		g_lock.lock();
		Find(key, &pred, &curr);
		if (key == curr[0]->key) {
			g_lock.unlock();
			return false;
		}
		else {
			// 큰 값 일수록 적은 확률 작은 값 일수록 높은 확률로 나와야 한다.
			// 배열에 미리 저장해두고 상수 시간으로 height를 얻게 하면 조금 더 좋아진다.
			// 랜덤 자료구조는 최적화를 위해 소요하는 행위가 오버헤드가 생길 우려가 더 크다. 랜덤 자료구조는 랜덤하게 냅두는 것이 제일 좋다.
			int height = 0;
			for (height; height < MAX_LEVEL - 1; ++height) if (rand() % 2 == 0) break;
			Node* node = new Node(key, height);
			for (auto i = 0; i < height + 1; ++i) {
				node->next[i] = curr[i]; // curr과 pred가 없는 높이를 만들거나, 같은 높이가 아님에도 연결되는 것을 방지 하기 위해
				pred[i]->next[i] = node; // cuur과 pred는 배열이 되어야 한다.
			}
			g_lock.unlock();
			return true;
		}
	}
	
	bool Remove(int key) {
		Nexts pred, curr;
		g_lock.lock();
		Find(key, &pred, &curr);
		if (key != curr[0]->key) {
			g_lock.unlock();
			return false;
		}
		else {
			for (int i = 0; i < curr[0]->top_level + 1; ++i) {
				pred[i]->next[i] = curr[i]->next[i];
			}
			delete curr[0];
			g_lock.unlock();
			return true;
		}
	}

	bool Contains(int key) {
		Nexts pred, curr;
		g_lock.lock();
		Find(key, &pred, &curr);
		if (key != curr[0]->key) {
			g_lock.unlock();
			return false;
		}
		else {
			g_lock.unlock();
			return true;
		}
	}

	void print20() {
		cout << "SKIP_LIST = ";
		Node *ptr = head.next[0];
		for (auto i = 0; i < 20; ++i) {
			if (&tail == ptr) break;
			cout << ptr->key << "[" << ptr->top_level <<"] : ";
			ptr = ptr->next[0];
		}
		cout << endl;
	}
};

CSKIP_LIST csklist;

void test_thread_func(int threa_num) {
	int key;
	for (auto i = 0; i < NUM_TEST / threa_num; ++i) {
		switch (rand() % 3) {
		case 0: key = rand() % KEY_RANGE;
			csklist.Add(key); break;
		case 1: key = rand() % KEY_RANGE;
			csklist.Remove(key); break;
		case 2: key = rand() % KEY_RANGE;
			csklist.Contains(key); break;
		default: cout << "Error" << endl;
			exit(-1);
		}
	}
}
int main() {
	std::vector<std::thread*> v_threadlist;
	v_threadlist.reserve(8);
	for (auto i = 1; i <= MAX_THREAD; i *= 2) {
		auto start_time = std::chrono::high_resolution_clock::now();
		csklist.Init();
		for (auto j = 0; j < i; ++j) {
			v_threadlist.push_back(new std::thread{ test_thread_func, i });
		}
		for each (auto th in v_threadlist) th->join();
		auto end_time = std::chrono::high_resolution_clock::now() - start_time;
		cout << "thread 개수 : " << i << endl;
		cout << std::chrono::duration_cast<std::chrono::milliseconds>(end_time).count() << "msecons" << endl;
		for each (auto th in v_threadlist) delete th;
		v_threadlist.clear();
		csklist.print20();
	}
	system("pause");
}