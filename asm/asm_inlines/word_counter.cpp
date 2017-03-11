#include <cstdint>
#include <emmintrin.h>
#include <stdio.h>
#include <iostream>

void print128_num_by_words(__m128i var)
{
    uint16_t *val = (uint16_t*) &var;
    printf("Value in words: %i %i %i %i %i %i %i %i \n", 
           val[0], val[1], val[2], val[3], val[4], val[5], 
           val[6], val[7]);
}

void print128_num_by_bytes(__m128i var)
{
    char *val = (char *) &var;
    printf("Value in bytes: ");
    for(size_t i = 0; i < 16; i++)
    {
        printf("%d ", (int)*(val + i));
    }
    printf("\n");
}
 
int count_words(char *arr, size_t size)
{
    __m128i space_mask_reg = _mm_set_epi8(32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32);
    __m128i shifted_mask_reg = _mm_set_epi8(255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    //////////////////////////////////////////////

    size_t cur_position = 0;
    size_t ans = 0;

    //todo why can't we make alignment by 8?????
    //making data aligned to use vectorization
    bool is_whitespace = false;
    while ((size_t) (arr + cur_position) % 16 != 0 && cur_position < size)
    {
        std::cerr << "have to move more " << 16 - (size_t) (arr + cur_position) % 16 << std::endl;

        char cur_symbol = *(arr + cur_position);
        if(is_whitespace && cur_symbol != ' ')
        {
            ans++;
        }

        is_whitespace = (cur_symbol == ' ');;
        cur_position++;
    }

    std::cerr << "\nNow is aligned, string left: " << std::endl;
    std::cerr << cur_position + arr;
    std::cerr << std::endl << std::endl;

    //main part, using vectorization, every step is 16 bytes
    for(size_t i = cur_position; i < size - cur_position - (size - cur_position) % 16; i += 16) //while i < new size floor 16
    {
        __m128i trasher;
        //compare value in mask register with 16
        //      bytes of given string, which is
        //      in memory and write result
        //      to cmp_result. Comparing by
        //      bytes!!                
        //here we write many asm inlines for debugging, later unite in one
        __m128i part_of_string;
        //mov part of string to register
        __asm__("movdqa\t" "(%1), %0"
                :"=x"(part_of_string)
                :"r"(arr + i));
        print128_num_by_bytes(part_of_string);

        __m128i cmp_result;
        //compare part of string with space mask
        __asm__("pcmpeqb\t" "%1, %0"
                :"=x"(cmp_result), "=x"(trasher)
                :"1"(part_of_string), "0"(space_mask_reg));
        print128_num_by_bytes(cmp_result); 

        //shift right???? by one
        __m128i cmp_result_sll_with_zero;
        __asm__("psrldq\t" "$1, %0"
                :"=x"(cmp_result_sll_with_zero)
                :"0"(cmp_result));

        __m128i cmp_result_shifted;
        __asm__("por\t" "%1, %0"
                :"=x"(cmp_result_shifted), "=x"(trasher)
                :"0"(cmp_result_sll_with_zero), "1"(shifted_mask_reg));
        print128_num_by_bytes(cmp_result_shifted);

        __m128i result; //on our byte there is space,
        //  but in right neigbour there is no space
        //do result = not cmp_res_shifted and cmp_res
        __asm__("pandn\t" "%1, %0"
                :"=x"(result), "=x"(trasher) //part of str is read not to get the same regs
                :"1"(cmp_result), "0"(cmp_result_shifted));
        print128_num_by_bytes(result);


    }

    std::cerr << "\nVectorization is over, doing the last part\n\n";

    //do the last third part
    //while ((size - cur_position) % 16 != 0)
    {

    }

    std::cerr << "then ans is: " << ans << std::endl;
} 

int main()
{
        char* str = (char*)"abc bbbbbb bbbbb bbbbbbbbbbbbbbbbb";
        char* str_strange = (char*)"wwwwwww        wwwwwwwwwww ww www        www w wwwwwwww"; //why segfault????
        char* str_strange1 = (char*)"aaaaaaaaaa _aaaaaaaaaaaaaaaaaaaa";


        //std::cout << (((size_t) str) % 16 == 0) << std::endl;
        //very very cool


        //count_words(str_strange, 16);
        count_words(str_strange, 55);
        return 0;
}