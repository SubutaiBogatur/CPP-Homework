//
// Created by Aleksandr Tukallo on 11.03.17.
//

#include <cstdint>
#include <emmintrin.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include "test_utils/test_utils.h"

bool logging_is_enabled = true;

#define LOG(x) do { \
  if (logging_is_enabled) { std::cerr << x; } \
} while (0)

std::string to_string_by_bytes(__m128i var, bool is_signed = false)
{
    std::string ret;
    ret += " in bytes: ";

    char *val = (char *) &var;
    for(size_t i = 0; i < 16; i++)
    {
        ret += std::to_string((is_signed ? (int)*(val + i) : (uint8_t)*(val + i))) + " ";
    }
    return ret;
}
 
uint32_t count_words(std::string const& str)
{
    char const *arr = str.c_str();
    size_t size = str.size();

    //not to put a lot of crutches when checking size all the time
    if(size <= 64)
    {
        return count_words_slowly(str);
    }

    __m128i space_mask_reg = _mm_set_epi8(32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32);

    size_t cur_position = 0;
    size_t ans = 0;

    LOG("\nString given:\n" + std::string(arr) + "\n\n");

    //making data aligned to use vectorization
    bool is_whitespace = false;
    while ((size_t) (arr + cur_position) % 16 != 0)
    {
        LOG("have to move more "  + std::to_string(16 - (size_t) (arr + cur_position) % 16) + "\n");

        char cur_symbol = *(arr + cur_position);
        if(is_whitespace && cur_symbol != ' ')
        {
            ans++;
        }

        is_whitespace = (cur_symbol == ' ');;
        cur_position++;
    }

    //some crutches
    if(cur_position == size) return ans;
    if ((is_whitespace && *(arr + cur_position) != ' ' && cur_position != 0)
                  || (cur_position == 0 && *arr != ' ')) ans++;
    if(cur_position != 0 && *arr != ' ') ans++;
    //

    LOG("\nNow is aligned, ans: " + std::to_string(ans) +  " string left:\n" + std::string(cur_position + arr) + "\n\n");

    //main part, using vectorization, every step is 16 bytes
    __m128i store = _mm_set_epi32(0, 0, 0, 0); //variable is used to store bytes got in `for` loop
    __m128i cur_cmp, next_cmp; //variables store current 16 bytes and next 16 bytes respectively after comparing them with space mask
    __m128i trasher; //todo delete later  

    //vectorization is done on indexes: [cur_position; (size - cur_position) % 16 - 16]
    size_t end_of_vectorization = size - (size - cur_position) % 16 - 16;
    
    //there is always current and next, because of size check
    //get first 16 bytes of string from memory and compare them with space mask
    __asm__("movdqa\t" "(%2), %1\n"
            "pcmpeqb\t" "%1, %0"
            :"=x"(next_cmp), "=x"(trasher)
            :"r"(arr + cur_position), "0"(space_mask_reg));
    

    for(size_t i = cur_position; i < end_of_vectorization; i += 16) //while i < new size (floor 16) - 16 (- 16 because, always reading next bytes)
    {
        //here we write many asm inlines for debugging, later unite in one

        cur_cmp = next_cmp;

        //get cmp for next part of string
        __asm__("movdqa\t" "(%2), %1\n"
                "pcmpeqb\t" "%1, %0"
                :"=x"(next_cmp), "=x"(trasher)
                :"r"(arr + i + 16), "0"(space_mask_reg));

        LOG("Curcmp" + to_string_by_bytes(cur_cmp) + "\n");
        LOG("Nexcmp" + to_string_by_bytes(next_cmp) + "\n");

        __m128i cmp_result_shifted;
        __asm__("palignr\t" "$1, %1, %0"
                :"=x"(cmp_result_shifted), "=x"(trasher)
                :"0"(next_cmp), "1"(cur_cmp));
        LOG("Shifte" + to_string_by_bytes(cmp_result_shifted) + "\n");

        //in our byte there is space,
        //  but in right neigbour there is no space (ie 1 if space before start of word)
        //result = not cmp_res_shifted and cmp_res
        //this result is summed and stored in store

        uint32_t msk;

       __asm__("pandn\t" "%3, %2\n"
                "psubsb\t" "%2, %4\n"
                "paddusb\t" "%4, %0\n"
                "pmovmskb\t" "%0, %1" 
                :"=x"(store), "=r"(msk) 
                :"x"(cmp_result_shifted), "x"(cur_cmp), "x"(_mm_set_epi32(0, 0, 0, 0)), "0"(store)
                :"2", "4");
       LOG("Store " + to_string_by_bytes(store) + "\n");

        //if at least one byte in store is more than 127
        //  or if it is last iteration of loop
        if(msk != 0 || i + 16 >= end_of_vectorization) 
        {
            //sum all bytes in store and then refresh it
            __m128i result_of_abs;
            __asm__("psadbw\t" "%1, %0"
                    :"=x"(result_of_abs), "=x"(trasher)
                    :"0"(_mm_set_epi32(0, 0, 0, 0)), "1"(store));
            LOG("Abs   " + to_string_by_bytes(result_of_abs, false) + "\n");

            //here we move lowest 32 bits (ie d) of abs to low.
            //  32 bits is enough, because max value of low can be:
            //  128 * 8 < 2 ^ 32
            uint32_t low;
            __asm__("movd\t" "%1, %0"
                    :"=r"(low)
                    :"x"(result_of_abs));
            LOG("low    " + std::to_string(low) + "\n");

            uint32_t high;
            __m128i abs_low;
            __asm__("movhlps\t" "%1, %0"
                    :"=x"(abs_low), "=x"(trasher)
                    :"1"(result_of_abs));
            LOG("LAbs  " + to_string_by_bytes(abs_low, false) + "\n");

            __asm__("movd\t" "%1, %0"
                    :"=r"(high)
                    :"x"(abs_low));
            LOG("high   " + std::to_string(high) + "\n");

            ans += high + low;
            store = _mm_set_epi32(0, 0, 0, 0);
        }

        LOG("\n");

    }
    cur_position = end_of_vectorization;

    LOG("\nVectorization is over, ans now: " + std::to_string(ans) + " doing the last part, string left:\n");
    LOG(std::string(arr + cur_position) + "\n\n");

    //some more crutches
    if(*(arr + cur_position - 1) == ' '
        && *(arr + cur_position) != ' ')
    {
        ans--;
    }

    is_whitespace = *(arr + cur_position - 1) == ' ';
    for(size_t i = cur_position; i < size; i++)
    {
        if (*(arr + i) != ' ' && is_whitespace) ans++;
        is_whitespace = *(arr + i) == ' ';
        LOG(std::to_string(size - i) + " chars left\n");
    }

    LOG("\nthen ans is: " + std::to_string(ans) + "\n");
    return ans;
} 

void do_tests(size_t num = 50000)
{
    std::srand(std::time(0));
    std::cout << std::time(0) << std::endl;
    for(size_t i = 0; i < num; i++)
    {
        std::string str = get_random_string(65, 7000, 2);
        std::cerr << "New string:\n" << str << std::endl << std::endl;
        uint32_t fast_ans = count_words(str);
        uint32_t slow_ans = count_words_slowly(str);

        std::cerr << "Slow ans: " << slow_ans << ", fast ans: " << fast_ans << std::endl; 
        if(slow_ans != fast_ans)
        {
            std::cin >> str;
        }
    }
}

void generate_large_input()
{
    std::ofstream ofs("test_utils/input_data.in");
    ofs << get_random_string(1e9, 2e9);
}

void measure_time(bool is_fast = true, uint32_t times = 10)
{
    std::ifstream ifs("test_utils/input_data.in");
    std::string s;
    std::getline(ifs, s);

    std::time_t start = clock();
    for(size_t i = 0; i < times; i++) 
    {
        is_fast ? count_words(s) : count_words_slowly(s);
    }
    std::cout << (clock() - start)/CLOCKS_PER_SEC << " seconds used\n";
}

int main()
{
        std::string str("abc bbbbbb bbbbb bbbbbbbbbbbbbbbbbbbb bbbbbbbbbbbbbbbbbbbb bbbbbbbbbbbbbbbbbbbb bbbbbbbbbbbbbbbbb");
        std::string str_strange("wwwwwww        wwwwwwwwwww ww www        www w wwwww     ww ww www        www w wwwww");
        std::string str_strange1("aaaaaaaaaa _aaaaaaaaaaaaaaaaaaaa");

        do_tests();

        return 0;
}