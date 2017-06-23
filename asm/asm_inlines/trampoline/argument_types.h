#ifndef ARGUMENT_TYPES_H
#define ARGUMENT_TYPES_H

template <typename ... Args>
struct args_types;

template <>
struct args_types<>
{
    static const int INTEGER = 0;
    static const int SSE = 0;
};

template <typename First, typename ... Args>
struct args_types<First, Args ...>
{
    static const int INTEGER = args_types<Args ...>::INTEGER + 1;
    static const int SSE = args_types<Args ...>::SSE;
};

template <typename ... Args>
struct args_types<float, Args ...>
{
    static const int INTEGER = args_types<Args ...>::INTEGER;
    static const int SSE = args_types<Args ...>::SSE + 1;
};

template <typename ... Args>
struct args_types<double, Args ...>
{
    static const int INTEGER = args_types<Args ...>::INTEGER;
    static const int SSE = args_types<Args ...>::SSE + 1;
};

#endif // ARGUMENT_TYPES_H

