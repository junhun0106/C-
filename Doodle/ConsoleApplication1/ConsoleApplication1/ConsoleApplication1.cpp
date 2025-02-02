﻿#include <iostream>
#include <ranges>
#include <vector>

int main() {
	std::vector<int> numbers = { 1,2,3,4,5,6 };

	auto results = numbers | std::views::filter([](int n) { return n % 2 == 0; })
		| std::views::transfrom([](int n) { return n * 2; });

	for (auto v : results) {
		std::cout << v << std::endl; // 4 8 12
	}
}