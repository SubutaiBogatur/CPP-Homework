
#include "test_utils.h"
#include <cstdlib>
#include <ctime>

std::string get_random_string(size_t min_len, size_t max_len, size_t number_of_letters_for_one_space)
{
	std::string ret;
	size_t len = std::rand() % (max_len - min_len) + min_len;

	for(size_t i = 0; i < len; i++)
	{
		//eg num = 2, then rand % 3 is zero once in 3 times, so 2 letters for one space
		bool is_space = std::rand() % (number_of_letters_for_one_space + 1) == 0;
		ret += is_space ? ' ' : 'a' + std::rand() % 26;
	}
	return ret;
}

uint32_t count_words_slowly(std::string const& str)
{
	uint32_t ret = 0;

	bool prev_is_space = true;
	for(size_t i = 0; i < str.size(); i++)
	{
		if(str[i] != ' ' && prev_is_space) //new word has started
		{
			ret++;
			prev_is_space = false;
		}
		else if(str[i] == ' ')
		{
			prev_is_space = true;
		}
	}

	return ret;
}