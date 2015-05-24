#include "segment_tree/segment_tree.hpp"
//#include "avl_array.hpp"

#include <chrono>
#include <deque>
#include <iostream>
#include <random>
#include <vector>

void spin(unsigned count)
{
	auto stop = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds{count};
	while (std::chrono::high_resolution_clock::now() < stop);
}

template<typename Functor>
float bench(Functor functor)
{
	auto start = std::chrono::high_resolution_clock::now();
	functor();
	auto stop = std::chrono::high_resolution_clock::now();
	auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
	return ns.count() / 1000000.0;
}

template<typename T>
void fill(std::default_random_engine & engine, T * first, T * last)
{
	std::uniform_int_distribution<T> distribution;

	while (first != last)
	{
		*first = distribution(engine);
		++first;
	}
}

template<template<typename> class Container, typename T>
void bench_insert(Container<T> & container)
{
	std::size_t const length = 17;
	std::size_t const count = 96000;
	std::default_random_engine engine;
	T buffer[length];

	auto ms = bench([&]
	{
		for (unsigned I = 0; I != count; ++I)
		{
			fill(engine, std::begin(buffer), std::end(buffer));
			std::uniform_int_distribution<std::size_t> distribution{0, container.size()};
			container.insert(
				container.begin() + distribution(engine),
//				container.index(distribution(engine)),
				buffer,
				buffer + length);
		}
	});

	std::cout << "\t" << ms << "ms insert\n" << std::flush;
}

template<template<typename> class Container, typename T>
void bench_accumulate(Container<T> & container)
{
	std::uint64_t constexpr prime = (1ULL << 32) - 5;
	std::uint64_t a = 1;
	std::uint64_t b = 0;

	auto ms = bench([&]
	{
		auto first = container.begin();
		auto last = container.end();
		while (first != last)
		{
			a = (a + *first) % prime;
			b = (b + a) % prime;
			++first;
		}
	});

	std::uint64_t total = (b << 32) | a;
	std::cout << "\t" << ms << "ms accumulate, " << total << "\n" << std::flush;
}

template<template<typename> class Container, typename T>
void bench_sort(Container<T> & container)
{
	auto ms = bench([&]
	{
		auto first = container.begin();
		auto last = container.end();
		std::sort(first, last);
//		container.sort();
	});
	std::cout << "\t" << ms << "ms sorting\n" << std::flush;
}

template<template<typename> class Container>
void bench_container(std::string const & description)
{
	std::cout << "begin " << description << "...\n" << std::flush;
	Container<std::size_t> container;
//	spin(1000);
	bench_insert(container);
//	spin(1000);
//	bench_accumulate(container);
//	spin(1000);
//	bench_sort(container);
//	spin(1000);
//	bench_accumulate(container);
}

template<typename T>
using vector = std::vector<T>;
template<typename T>
using deque = std::deque<T>;
template<typename T>
using segment = segment_tree_t<T>;
//template<typename T>
//using avl = mkr::avl_array<T>;

int main()
{
	bench_container<vector>("vector");
//	bench_container<deque>("deque");
	bench_container<segment>("segment_tree");
//	bench_container<avl>("avl_array");
}
