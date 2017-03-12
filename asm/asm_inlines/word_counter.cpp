#include <cstdint>
#include <emmintrin.h>
#include <stdio.h>
#include <iostream>
#include <string>

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
 
void count_words(char *arr, size_t size)
{
    __m128i space_mask_reg = _mm_set_epi8(32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32);
    __m128i shifted_mask_reg = _mm_set_epi8(255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    //////////////////////////////////////////////

    size_t cur_position = 0;
    size_t ans = 0;

    LOG("\nString given:\n" + std::string(arr) + "\n\n");

    //making data aligned to use vectorization
    bool is_whitespace = false;
    while ((size_t) (arr + cur_position) % 16 != 0 && cur_position < size)
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
    if ((is_whitespace && *(arr + cur_position) != ' ' && cur_position != 0)
                  || (cur_position == 0 && *arr != ' ')) ans++;
    if(cur_position != 0 && *arr != ' ') ans++;
    //

    LOG("\nNow is aligned, string left:\n" + std::string(cur_position + arr) + "\n\n");

    //main part, using vectorization, every step is 16 bytes
    __m128i store; //variable is used to store bytes got in `for` loop
    store = _mm_set_epi32(0, 0, 0, 0);
    for(size_t i = cur_position; i < size - cur_position - (size - cur_position) % 16; i += 16) //while i < new size floor 16
    {
        __m128i trasher; //todo delete later         
        //here we write many asm inlines for debugging, later unite in one

        __m128i part_of_string;
        //mov part of string to register
        __asm__("movdqa\t" "(%1), %0"
                :"=x"(part_of_string)
                :"r"(arr + i));

        __m128i cmp_result;
        //compare part of string with space mask
        __asm__("pcmpeqb\t" "%1, %0"
                :"=x"(cmp_result), "=x"(trasher)
                :"1"(part_of_string), "0"(space_mask_reg));

        //shift right???? by one
        __m128i cmp_result_sll_with_zero;
        __asm__("psrldq\t" "$1, %0"
                :"=x"(cmp_result_sll_with_zero)
                :"0"(cmp_result));

        __m128i cmp_result_shifted;
        __asm__("por\t" "%1, %0"
                :"=x"(cmp_result_shifted), "=x"(trasher)
                :"0"(cmp_result_sll_with_zero), "1"(shifted_mask_reg));

        __m128i result; //in our byte there is space,
        //  but in right neigbour there is no space (ie 1 if space before start of word)
        //result = not cmp_res_shifted and cmp_res
        __asm__("pandn\t" "%1, %0"
                :"=x"(result), "=x"(trasher) 
                :"1"(cmp_result), "0"(cmp_result_shifted));

        //result = _mm_set_epi8(50, 20, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        LOG("Result" + to_string_by_bytes(result, true) + "\n");

        __m128i positive_result; //just all -1 become 1 to use unsigned saturation addition later
        __asm__("psubsb\t" "%1, %0"
                :"=x"(positive_result), "=x"(trasher)
                :"0"(_mm_set_epi32(0, 0, 0, 0)), "1"(result));
        LOG("Positi" + to_string_by_bytes(positive_result, true) + "\n");

        __asm__("paddusb\t" "%1, %0"
                :"=x"(store), "=x"(trasher)
                :"1"(positive_result), "0"(store));
        LOG("Store " + to_string_by_bytes(store, false) + "\n");

        uint32_t msk;
        __asm__("pmovmskb\t" "%1, %0"
            :"=r"(msk), "=x"(trasher)
            :"x"(store), "0"(msk));
        LOG("Moscow " + std::to_string(msk) + "\n");

        if(msk != 0) //if at least one byte in store is more than 128
        {
            //sum all bytes in store and then refresh it
            __m128i result_of_abs;
            __m128i zero = _mm_set_epi32(0, 0, 0, 0);
            __asm__("psadbw\t" "%1, %0"
                    :"=x"(result_of_abs), "=x"(trasher)
                    :"0"(zero), "1"(store));
            std::cerr << "inside";

            //how to
            //now we want to get sum of upper and lower quadwords:  
            /*__m128i lower;
            __asm__("pextrq\t" "%1, %0, $0"
                    :"=x"(lower)
                    :"x"(result_of_abs));*/
        }

    }

    std::cerr << "\nVectorization is over, doing the last part, string left:\n";
    std::cerr << arr + size - cur_position - (size - cur_position) % 16 << std::endl << std::endl;

    //do the last third part
    //while ((size - cur_position) % 16 != 0)
    {

    }

    std::cerr << "then ans is: " << ans << std::endl;
} 

int main()
{
        char* str = (char*)"abc bbbbbb bbbbb bbbbbbbbbbbbbbbbb";
        char* str_strange = (char*)"wwwwwww        wwwwwwwwwww ww www        www w wwwwwwwwwww        www w wwwwwww        www w wwwwwwww        www w wwwwwww        www w wwww"; //why segfault????
        char* str_strange1 = (char*)"aaaaaaaaaa _aaaaaaaaaaaaaaaaaaaa";


        //std::cout << (((size_t) str) % 16 == 0) << std::endl;
        //very very cool


        //count_words(str_strange, 16);
        count_words(str_strange, 140);
        return 0;
}