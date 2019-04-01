//
// Created by atukallo on 4/1/19.
//

#ifndef CONTROL_2_PARSER_COMBINATORS_HPP
#define CONTROL_2_PARSER_COMBINATORS_HPP

#include <optional>
#include <string_view>
#include <utility>

namespace control_2 {

    template<typename T>
    struct result {
    private:
        std::optional<T> parsed_value;
        std::string_view remaining_part;

    public:
        result(std::optional<T> parsed_value, std::string_view remaining_part) : parsed_value(std::move(parsed_value)),
                                                                                 remaining_part(remaining_part) {}

        // constructs result with no value
        explicit result(std::string_view remaining_part) : parsed_value(), remaining_part(remaining_part) {}

        std::string_view remaining() const {
            return remaining_part;
        }

        T value() const {
            if (!parsed_value.has_value()) {
                throw std::runtime_error("no value");
            }

            return parsed_value.value();
        }

        operator bool() const {
            return parsed_value.has_value();
        }
    };

    struct ch {
    public:
        using parsed_type = char;

        explicit ch(parsed_type value) : value(value) {}

        result<parsed_type> parse(std::string_view str) const {
            if (str.empty() || str[0] != value) {
                return result<parsed_type>(str);
            }
            return result<parsed_type>(value, str.substr(1, str.size() - 1));
        };
    private:
        const parsed_type value;
    };

    struct str {
        using parsed_type = std::string;

        explicit str(parsed_type value) : value(std::move(value)) {}

        result<parsed_type> parse(std::string_view str) const {
            if (str.size() < value.size() || value != str.substr(0, value.size())) {
                return result<parsed_type>(str);
            }

            return result<parsed_type>(value, str.substr(value.size(), str.size() - value.size()));
        };

    private:
        const parsed_type value;
    };

    template<class Parser>
    struct opt {
    public:
        using parsed_type = std::optional<typename Parser::parsed_type>;

        explicit opt(Parser parser) : parser(parser) {}

        result<parsed_type> parse(std::string_view str) const {
            auto opt_result = parser.parse(str);
            auto parsed_value = std::optional<typename Parser::parsed_type>();
            if (opt_result) {
                parsed_value = std::make_optional<typename Parser::parsed_type>(opt_result.value());
            }
            return result<parsed_type>(parsed_value, opt_result.remaining());
        }

    private:
        const Parser parser;
    };

    template<class Parser>
    struct many {
    public:
        using parsed_type = std::vector<typename Parser::parsed_type>;

        explicit many(Parser parser) : parser(parser) {}

        result<parsed_type> parse(std::string_view str) const {
            std::string_view cur_str = str;
            std::vector<typename Parser::parsed_type> ret;
            while (true) {
                auto cur_result = parser.parse(cur_str);
                if (!cur_result || cur_result.remaining().size() == cur_str.size()) {
                    return result<parsed_type>(ret, cur_result.remaining());
                } else {
                    cur_str = cur_result.remaining();
                    ret.emplace_back(cur_result.value());
                }
            }
            // cannot loop infinitely, bc every time remaining reduces
        }

    private:
        const Parser parser;
    };

    template<class Parser, class Transformer>
    struct rule {
    public:
        using parsed_type = decltype(std::declval<Transformer>()(std::declval<typename Parser::parsed_type>()));

        rule(Parser parser, Transformer trans) : parser(parser), trans(trans) {}

        result<parsed_type> parse(std::string_view str) const {
            auto parser_result = parser.parse(str);
            if (parser_result) {
                return result<parsed_type>(trans(parser_result.value()), parser_result.remaining());
            } else {
                return result<parsed_type>(parser_result.remaining());
            }
        }

    private:
        const Parser parser;
        const Transformer trans;
    };

    template<class Parser, class... Parsers>
    struct alt : alt<Parsers...> {
    public:
        using parsed_type = typename alt<Parsers...>::parsed_type;

        explicit alt(Parser parser, Parsers... parsers) : alt<Parsers...>(parsers...), parser(parser) {}

        result<parsed_type> parse(std::string_view str) const {
            auto parser_result = parser.parse(str);
            if (parser_result) {
                return result<parsed_type>(parser_result.value(), parser_result.remaining());
            } else {
                return alt<Parsers...>::parse(str);
            }
        }

    private:
        const Parser parser;
    };

    template<class Parser>
    struct alt<Parser> {
    public:
        using parsed_type = typename Parser::parsed_type;

        explicit alt(Parser parser) : parser(parser) {}

        result<parsed_type> parse(std::string_view str) const {
            auto parser_result = parser.parse(str);
            if (parser_result) {
                return result<parsed_type>(parser_result.value(), parser_result.remaining());
            } else {
                return result<parsed_type>(parser_result.remaining());
            }
        }

    private:
        const Parser parser;
    };
}

#endif //CONTROL_2_PARSER_COMBINATORS_HPP
