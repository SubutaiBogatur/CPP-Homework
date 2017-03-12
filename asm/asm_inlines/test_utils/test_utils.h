//
// Created by Aleksandr Tukallo on 12.03.17.
//

#ifndef TEST_UTILS
#define TEST_UTILS

#include <string>

//function returns random string with English lower-case letters
//	and spaces. 
std::string get_random_string(size_t min_len = 5, size_t max_len = 50, size_t number_of_letters_for_one_space = 5);

uint32_t count_words_slowly(std::string const& str);


#endif //TEST_UTILS