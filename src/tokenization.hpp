#pragma once

enum class TokenType {exit, int_lit, semi, open_paren, close_paren, ident, may, equal};

struct Token {
    TokenType type;
    std::optional<std::string> value {};
};

class Tokenizer {

    public:
        inline explicit Tokenizer(const std::string& src) : m_src(move(src)) {

        }

        inline std::vector<Token> tokenize() {
            std::vector<Token> tokens;
            std::string buf;

            while (peek().has_value()) {
                if (std::isalpha(peek().value())) {
                    buf.push_back(engulf());

                    while (peek().has_value() && std::isalnum(peek().value())) {
                        buf.push_back(engulf());
                    }

                    if (buf == "exit") {
                        tokens.push_back({.type = TokenType::exit});
                        buf.clear();
                        continue;
                    } else if (buf == "may") {
                        tokens.push_back({.type = TokenType::may});
                        buf.clear();
                        continue;
                    } else {
                        tokens.push_back({.type = TokenType::ident, .value = buf});
                        buf.clear();
                        continue;
                    }
                } else if (std::isdigit(peek().value())) {
                    buf.push_back(engulf());
                    while (peek().has_value() && std::isdigit(peek().value())) {
                        buf.push_back(engulf());
                    }

                    tokens.push_back({.type = TokenType::int_lit, .value = buf});
                    buf.clear();
                    continue;
                } else if (peek().value() == '(') {
                    engulf();
                    tokens.push_back({.type = TokenType::open_paren});
                    continue;
                } else if (peek().value() == ')') {
                    engulf();
                    tokens.push_back({.type = TokenType::close_paren});
                    continue;
                }else if (peek().value() == ';') {
                    engulf();
                    tokens.push_back({.type = TokenType::semi});
                    continue;
                } else if (peek().value() == '=') {
                    engulf();
                    tokens.push_back({.type = TokenType::equal});
                    continue;
                }
                else if (std::isspace(peek().value())) {
                    engulf();
                    continue;
                } else {
                    std::cerr << "You messed up!" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }

            m_index = 0;
            return tokens;
        }

    private:
        const std::string m_src;
        size_t  m_index = 0;

         [[nodiscard]] inline std::optional<char> peek(int offset = 0) const {
            if (m_index + offset >= m_src.length()) {
                return  {};
            } else
                return m_src[m_index + offset];
        }

        inline char engulf() {
            return m_src[m_index++];
        }
};