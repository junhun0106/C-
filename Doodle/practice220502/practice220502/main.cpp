#include <ranges>
#include <vector>
#include <iostream>
#include <algorithm>
#include <list>

class A 
{
public:
    int Value;

    A(int value) {
        Value = value;
    }
};

void Set(int* i) {
    std::cout << i << std::endl;
    std::cout << &i << std::endl;
    *i = 1;
}

void Set2(int& i) {
    std::cout << i << std::endl;
    std::cout << &i << std::endl;
    i = 2;
}

int main()
{
    int i = 0;
    Set(&i);
    std::cout << &i << std::endl;
    
    Set2(i);
    std::cout << i << std::endl;


    std::vector<A*> v;

    v.push_back(new A(1));
    v.push_back(new A(2));
    v.push_back(new A(3));

    for (int i = 0; i < v.size(); ++i) {
        auto value = v[i];
        std::cout << value << "," << value->Value << std::endl;
    }

    for (auto value : v) {
        std::cout << value << "," << value->Value << std::endl;

        delete value;
    }

    for (auto value : v) {
        std::cout << &value << std::endl;
    }
}