#include <iostream>
#include <string>
#include <emmintrin.h>
#include <ctime>
#include "test_utils/test_utils.h"

bool logging_is_enabled = false;

#define LOG(x) do { \
  if (logging_is_enabled) { std::cerr << x; } \
} while (0)

void naive_memcpy(void *dst, void const *src, size_t size)
{
	for(size_t i = 0; i < size; i++)
	{
		*((char *)dst + i) = *((char *)src + i);
	}
}

void vectorized_memcpy(void *dst, void const *src, size_t size)
{
	if(size < 32) 
	{
		return naive_memcpy(dst, src, size);
	}

	size_t offset = 0;
	while(((size_t)dst + offset) % 16 != 0)
	{
		*((char *)dst + offset) = *((char *)src + offset);
		offset++;
	}

	size_t end_of_vectorization = (size - offset) % 16;
	for(size_t i = offset; i < size - end_of_vectorization; i += 16)	
	{
		__m128i tmp;
		__asm__ (
		"movdqu (%1), %0\n"
		"movntdq %0, (%2)\n"
		:"=x"(tmp) //to specify register size explicitly
		:"r"((char *)src + i), "r"((char *)dst + i));
	}

	for (size_t i = size - end_of_vectorization; i < size; i++)
	{
        *((char *)dst + i) = *((char *)src + i);
    }
	_mm_sfence();
}

void do_tests(size_t num = 500)
{
    time_t seed = std::time(0);
    std::cerr << "Seed: " << seed << std::endl;
    std::srand(seed);

    clock_t slow_clocks = 0, fast_clocks = 0;

    int visualization_step = num / 100;
    int prev_step = 0;

    std::cerr << "0% done\n";

    for(size_t i = 0; i < num; i++)
    {
        if (i == prev_step + visualization_step)
        {
            prev_step = i;
            std::cerr << prev_step / visualization_step << "% done\n";
        }

        std::string str = get_random_string(1e4, 2e5, 2);

        char* buffer1 = new char[str.size()];
        char* buffer2 = new char[str.size()];

        LOG("Current test: " + str + "\n");

        clock_t begin = clock();
        vectorized_memcpy(buffer1, str.c_str(), str.size());
        fast_clocks += (clock() - begin);

        begin = clock();
        naive_memcpy(buffer2, str.c_str(), str.size());
        slow_clocks += (clock() - begin);

        if(!check_equality_memcpy(buffer1, buffer2, str.size()))
        {
            std::cin >> str;
        }
        else
        {
	        LOG("Test done and is succesful\n");
        }

        delete[] buffer1;
        delete[] buffer2;
    }
    std::cerr << num << " tests done, my congratulations.\n" 
    << "Fast counter has taken " << ((double)fast_clocks) / CLOCKS_PER_SEC 
    << ", while slow: " << ((double)slow_clocks) / CLOCKS_PER_SEC << ".\n" 
    << "Optimized code is " << (double) slow_clocks / (double) fast_clocks << " times faster\n";
}

int main()
{
	// std::string a = "abcdefghijadslkfjaslkdfja;ldsjflasdkjf;alkdjfa;ldkjfal;sdkjfa;lsdkfja;lsdkfja;lsdkjfa;ldkfja;lsdkfja;lsdkfja;ldkfja;ldkfj;adkfj;aldkfja;kdlfja;lkdfja;ldkfja;dklfj";
	// char *buffer = new char[a.size()];
	// vectorized_memcpy(buffer, a.c_str(), a.size());
	// std::cout << buffer << std::endl;

	do_tests();

	// delete[] buffer;
}