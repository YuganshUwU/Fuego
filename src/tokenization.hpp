#pragma once

enum class TokenType {exit, int_lit, semi, open_paren, close_paren, ident, may,
                      equal, plus, star, minus, fslash, curly_open, curly_close,
                      if_, elif, else_ };

inline std::optional<int> bin_prec(const TokenType type) {
    switch (type) {
        case TokenType::star:
        case TokenType::fslash:
            return 1;
        case TokenType::plus:
        case TokenType::minus:
            return 0;
        default:
            return {};
    }
}

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
                if (std::isalpha(peek().value()) || peek().value() == '_') {
                    buf.push_back(engulf());

                    while (peek().has_value() && std::isalnum(peek().value()) || peek().value() == '_') {
                        buf.push_back(engulf());
                    }

                    if (buf == "exit") {
                        tokens.push_back({.type = TokenType::exit});
                        buf.clear();
                    } else if (buf == "may") {
                        tokens.push_back({.type = TokenType::may});
                        buf.clear();
                    } else if (buf == "if") {
                        tokens.push_back({.type = TokenType::if_});
                        buf.clear();
                    } else if (buf == "elif") {
                        tokens.push_back({.type = TokenType::elif});
                        buf.clear();
                    } else if (buf == "else") {
                        tokens.push_back({.type = TokenType::else_});
                        buf.clear();
                    } else {
                        tokens.push_back({.type = TokenType::ident, .value = buf});
                        buf.clear();
                    }
                } else if (std::isdigit(peek().value())) {
                    buf.push_back(engulf());
                    while (peek().has_value() && std::isdigit(peek().value())) {
                        buf.push_back(engulf());
                    }

                    tokens.push_back({.type = TokenType::int_lit, .value = buf});
                    buf.clear();
                } else if (peek().value() == '-' && peek(1).has_value() && peek(1).value() == '-') {
                    engulf();
                    engulf();

                    while (peek().has_value() && peek().value() != '\n') {
                        engulf();
                    }
                    engulf();
                } else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*') {
                    engulf();
                    engulf();

                    while (peek().has_value()) {
                        if (peek().value() == '*' && peek(1).has_value() && peek(1).value() == '/')
                            break;

                        engulf();
                    }

                    if (peek().has_value())
                        engulf();
                    if (peek().has_value())
                        engulf();
                } else if (peek().value() == '(') {
                    engulf();
                    tokens.push_back({.type = TokenType::open_paren});
                } else if (peek().value() == ')') {
                    engulf();
                    tokens.push_back({.type = TokenType::close_paren});
                }else if (peek().value() == ';') {
                    engulf();
                    tokens.push_back({.type = TokenType::semi});
                } else if (peek().value() == '=') {
                    engulf();
                    tokens.push_back({.type = TokenType::equal});
                } else if (peek().value() == '+') {
                    engulf();
                    tokens.push_back({.type = TokenType::plus});
                } else if (peek().value() == '*') {
                    engulf();
                    tokens.push_back({.type = TokenType::star});
                } else if (peek().value() == '/') {
                    engulf();
                    tokens.push_back({.type = TokenType::fslash});
                } else if (peek().value() == '-') {
                    engulf();
                    tokens.push_back({.type = TokenType::minus});
                } else if (peek().value() == '{') {
                    engulf();
                    tokens.push_back({.type = TokenType::curly_open});
                } else if (peek().value() == '}') {
                    engulf();
                    tokens.push_back({.type = TokenType::curly_close});
                } else if (std::isspace(peek().value())) {
                    engulf();
                } else {
                    std::cerr << "You messed up in tokenization!" << std::endl;
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