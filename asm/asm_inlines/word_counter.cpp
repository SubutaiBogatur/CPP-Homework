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
        __m128i space_mask_reg;

        //firstly generate space mask (ie 16 bytes of spaces)
        //      and put it into space_mask_reg variable
        //its preprocessing, absolutely temporary function,
        //      which is not needed in final implementation.
        //      Mb just one mov can be made
        {
                char* space_mask = new char[16];
                for (size_t i = 0; i < 16; i++)
                {
                        space_mask[i] = ' ';
                }
        
                __asm__("movdqa\t" "(%1), %0"
                        :"=x"(space_mask_reg) //take result to space_mask from 128 bit xmm register
                        :"r"(space_mask));
                delete[] space_mask;
        }
        //print128_num_by_bytes(space_mask_reg);
        //mask generated

        __m128i shifted_mask_reg;
        //generate mask 7 zero bytes and last all ones byte
        {
                char* shifted_mask = new char[16];
                for (size_t i = 0; i < 15; i++)
                {
                        shifted_mask[i] = 0;
                }
                shifted_mask[15] = 255;

                __asm__("movdqa\t" "(%1), %0"
                        :"=x"(shifted_mask_reg) //take result to space_mask from 128 bit xmm register
                        :"r"(shifted_mask));
                delete[] shifted_mask;
        }
        //print128_num_by_bytes(shifted_mask_reg);

        //////////////////////////////////////////////

        //first work when size is multiplier of 16
        for(size_t i = 0; i < size - size % 16; i += 16)
        {
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
                        :"=x"(cmp_result), "=x"(space_mask_reg)
                        :"1"(part_of_string), "0"(space_mask_reg));
                print128_num_by_bytes(cmp_result); 

                //shift right???? by one
                __m128i cmp_result_sll_with_zero;
                __asm__("psrldq\t" "$1, %0"
                        :"=x"(cmp_result_sll_with_zero)
                        :"0"(cmp_result));
                print128_num_by_bytes(cmp_result_sll_with_zero);

                __m128i cmp_result_shifted;
                __asm__("por\t" "%1, %0"
                        :"=x"(cmp_result_shifted), "=x"(shifted_mask_reg)
                        :"0"(cmp_result_sll_with_zero), "1"(shifted_mask_reg));
              print128_num_by_bytes(cmp_result_shifted);
      }
} 

int main()
{
        char* str = "abc bbbbbbbbbbb bbbbbbbbbbbbbbbbb";
        char* str_strange = "wwwwwwwwwwwwwwww"; //why segfault????

        count_words(str, 16);
        return 0;
}