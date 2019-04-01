#include <string>
#include <vector>
#include <cassert>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <functional>
#include <iostream>

#include "parser_combinators.hpp"

#define TASK_0
#define TASK_1

#define TASK_1_OPT
#define TASK_1_MANY

#define TASK_2

#define TASK_2_RULE
#define TASK_2_ALT

//#define TASK_3

namespace control_2 {}

template<typename Result>
void assert_no_value(Result const& result) {
    try {
        result.value();
        assert(false);
    } catch (std::runtime_error const& e) {

    }
}

#ifdef TASK_2_ALT
auto const DIGITS = control_2::alt(
    control_2::ch('0'),
    control_2::ch('1'),
    control_2::ch('2'),
    control_2::ch('3'),
    control_2::ch('4'),
    control_2::ch('5'),
    control_2::ch('6'),
    control_2::ch('7'),
    control_2::ch('8'),
    control_2::ch('9')
);
#endif

void test_basic_blocks() {
#ifdef TASK_0
    {
        auto const ch_parser = control_2::ch('a');

        {
            auto const result = ch_parser.parse("a");

            assert(result); // result converts to bool
            assert(result.remaining().empty());
            assert(result.value() == 'a');
        }

        {
            auto const result = ch_parser.parse("ab");

            assert(result); // result converts to bool
            assert(result.remaining() == "b");
            assert(result.value() == 'a');
        }

        {
            auto const result = ch_parser.parse("ba");

            assert(!result); // result converts to bool
            assert(result.remaining() == "ba");

            assert_no_value(result);
        }

        {
            auto const result = ch_parser.parse("");

            assert(!result); // result converts to bool
            assert(result.remaining() == "");

            assert_no_value(result);
        }
    }

    {
        auto const str_parser = control_2::str("hello");

        {
            auto const result = str_parser.parse("hello");

            assert(result);
            assert(result.remaining().empty());
            assert(result.value() == "hello");
        }

        {
            auto const result = str_parser.parse("hellohello");

            assert(result);
            assert(result.remaining() == "hello");
            assert(result.value() == "hello");
        }

        {
            auto const result = str_parser.parse("hekkohello");

            assert(!result);
            assert(result.remaining() == "hekkohello");
            assert_no_value(result);
        }

        {
            auto const result = str_parser.parse("");
            assert(!result);
            assert(result.remaining().empty());
            assert_no_value(result);
        }
    }
#endif
}

void test_simple_combinators() {
#ifdef TASK_1

#ifdef TASK_1_OPT
    {
        {
            auto const option_string = control_2::opt(control_2::str("hello"));

            auto const result = option_string.parse("hello again");

            assert(result);
            assert(result.remaining() == " again");
            assert(result.value());
            assert(*result.value() == "hello");
        }

        {
            auto const option_char = control_2::opt(control_2::ch('g'));

            auto const result = option_char.parse("again");

            assert(result);
            assert(result.remaining() == "again");
            assert(!result.value());
        }
    }
#endif

#ifdef TASK_1_MANY
    {
        auto const many_hellos = control_2::many(control_2::str("one"));

        {
            auto const result = many_hellos.parse("oneoneone");

            assert(result);
            assert(result.remaining().empty());
            assert((result.value() == std::vector<std::string>{"one", "one", "one"}));
        }

        {
            auto const result = many_hellos.parse("oneonetwo");

            assert(result);
            assert(result.remaining() == "two");
            assert((result.value() == std::vector<std::string>{"one", "one"}));
        }

        {
            auto const result = many_hellos.parse("on");

            assert(result);
            assert(result.remaining() == "on");
            assert(result.value().empty());
        }
    }

    {
//        auto const many_manys = control_2::many(control_2::many(control_2::ch('+')));
        auto const many_manys = control_2::many<control_2::many<control_2::ch>>(control_2::many(control_2::ch('+')));

        {
            auto const res = many_manys.parse("+++++");

            assert(res);
            assert(res.remaining().empty());

            // second run of many(ch('+')) did not progress, so many(many(ch('+')) stops parsing
            assert((res.value() == std::vector<std::vector<char>>{std::vector<char>(5, '+')}));
        }

        {
            auto const res = many_manys.parse("-+++++");

            assert(res);
            assert(res.remaining() == "-+++++");
            assert(res.value().empty());
        }
    }
#endif

#if defined(TASK_1_MANY) && defined(TASK_1_OPT)
    {
        {
            auto const opt_of_many = control_2::opt(control_2::many(control_2::ch('0')));
            // нет смысла с точки зрения парсинга
            auto const res = opt_of_many.parse("000001");

            assert(res);
            assert(res.remaining() == "1");
            assert(res.value());
            assert(*res.value() == std::vector<char>(5, '0'));
        }

        {
            auto const many_of_opt = control_2::many(control_2::opt(control_2::ch('0')));

            auto const res = many_of_opt.parse("000001");

            assert(res);
            assert(res.remaining() == "1");
            assert(res.value() == (std::vector<std::optional<char>>{{'0'}, {'0'}, {'0'}, {'0'}, {'0'}}));
        }
    }
#endif

#endif
}

void test_rule_and_alt() {
#ifdef TASK_2

#ifdef TASK_2_RULE
    {
        auto const rule_parser = control_2::rule(
            control_2::many(control_2::ch('g')),
            [](std::vector<char> const& v) -> size_t { return v.size(); }
        );

        {
            auto const result = rule_parser.parse("hello");

            assert(result);
            assert(result.remaining() == "hello");
            assert(result.value() == 0);
        }

        {
            auto const result = rule_parser.parse("gggaggg");

            assert(result);
            assert(result.remaining() == "aggg");
            assert(result.value() == 3);
        }
    }
#endif

#ifdef TASK_2_ALT
    {
        auto const alt_parser = control_2::alt(control_2::ch('+'), control_2::ch('-'));

        {
            auto const result = alt_parser.parse("+");

            assert(result);
            assert(result.remaining().empty());
            assert(result.value() == '+');
        }

        {
            auto const result = alt_parser.parse("-");

            assert(result);
            assert(result.remaining().empty());
            assert(result.value() == '-');
        }

        {
            auto const result = alt_parser.parse("*");

            assert(!result);
            assert(result.remaining() == "*");
            assert_no_value(result);
        }
    }

    {
        auto const alt_string_parser = control_2::alt(
            control_2::str("hello"),
            control_2::str("hi"),
            control_2::str("sup")
        );

        {
            assert(alt_string_parser.parse("hello, friend"));
            assert(alt_string_parser.parse("hi, friend"));
            assert(!alt_string_parser.parse("welcome"));
        }

        {
            auto const alt_string_parser_ext = control_2::alt(control_2::str("welcome"), alt_string_parser);

            assert(alt_string_parser_ext.parse("hello, friend"));
            assert(alt_string_parser_ext.parse("hi, friend"));
            assert(alt_string_parser_ext.parse("welcome"));
        }
    }
#endif

#if defined(TASK_2_ALT) && defined(TASK_2_RULE)
    {
        auto const rule_and_alt = control_2::alt(
            control_2::rule(control_2::str("hello"), [](std::string const&) -> std::string {
                return "string hello";
            }),
            // no seq at this point
            control_2::rule(control_2::many(DIGITS), [](std::vector<char> const& v) -> std::string {
                std::string number;
                number.resize(v.size());
                number.insert(0, v.data(), v.size());
                return "number " + std::to_string(std::atoi(number.c_str()));
            })
        );

        assert(rule_and_alt.parse("1234").value() == "number 1234");
        assert(rule_and_alt.parse("hello").value() == "string hello");
    }

#endif

#endif
}

void test_seq() {
#ifdef TASK_3

    {
        auto const seq_parser = control_2::seq(
            control_2::str("hello")
        );

        {
            auto const result = seq_parser.parse("hello");

            assert(result);
            assert(result.remaining().empty());
            assert(result.value() == std::make_tuple("hello"));
        }
    }

    {
        auto const seq_parser = control_2::seq(
            control_2::str("hello"),
            control_2::many(control_2::ch(' ')),
            control_2::str("world"),
            control_2::opt(control_2::ch('!'))
        );

        {
            auto const result = seq_parser.parse("hello     world!");

            assert(result);
            assert(result.remaining().empty());
            assert((result.value()
                    ==
                    std::make_tuple("hello", std::vector<char>(5, ' '), "world", std::optional{'!'}))
            );
        }

        {
            auto const result = seq_parser.parse("helloworld");

            assert(result);
            assert(result.remaining().empty());
            assert((result.value() == std::make_tuple("hello", std::vector<char>{}, "world", std::nullopt)));
        }

        {
            auto const result = seq_parser.parse("hello world!!");

            assert(result);
            assert(result.remaining() == "!");
            assert((result.value() == std::make_tuple("hello", std::vector<char>{' '}, "world", std::optional{'!'})));
        }

        {
            auto const result = seq_parser.parse("not hello world");

            assert(!result);
            assert(result.remaining() == "not hello world");
            assert_no_value(result);
        }
    }
#endif
}

void test_complex_parsing() {
#ifdef TASK_3
    auto const at_least_one_digit = control_2::rule(
        control_2::seq(DIGITS, control_2::many(DIGITS)),
        [](std::tuple<char, std::vector<char>> const& res) -> std::string {
            std::string result(std::get<1>(res).size() + 1, ' ');

            result[0] = std::get<0>(res);
            result.insert(1, std::get<1>(res).data(), std::get<1>(res).size());

            return result;
        }
    );

    auto const number = control_2::rule(at_least_one_digit, [](std::string const& s) -> int64_t {
        return std::atoi(s.c_str());
    });

    assert(number.parse("123").value() == 123);
    assert(number.parse("1024").value() == 1024);

    auto const sign = control_2::alt(control_2::ch('+'), control_2::ch('-'));

    auto const expr = control_2::rule(control_2::seq(number, control_2::many(control_2::seq(sign, number))),
                                      [](auto const& res) {
                                          auto&[first, rest] = res;

                                          auto result = first;

                                          for (auto&[sign, number]: rest) {
                                              if (sign == '+') {
                                                  result += number;
                                              }

                                              if (sign == '-') {
                                                  result -= number;
                                              }
                                          }

                                          return result;
                                      });

    assert(expr.parse("2+3-4").value() == 1);
    assert(expr.parse("1024-256-256").value() == 512);

    assert(!expr.parse("(not expression)"));
#endif
}

int main() {
    test_basic_blocks();
    test_simple_combinators();

    test_rule_and_alt();

    test_seq();

    test_complex_parsing();

    std::cout << "passed all tests!" << std::endl;

    return 0;
}