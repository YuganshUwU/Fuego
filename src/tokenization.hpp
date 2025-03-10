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
    int line;
    std::optional<std::string> value {};
};

class Tokenizer {

    public:
    explicit Tokenizer(const std::string& src) : m_src(move(src)) {

        }

        std::vector<Token> tokenize() {
            std::vector<Token> tokens;
            std::string buf;
            int line_count = 1;

            while (peek().has_value()) {
                if (std::isalpha(peek().value()) || peek().value() == '_') {
                    buf.push_back(engulf());

                    while (peek().has_value() && std::isalnum(peek().value()) || peek().value() == '_') {
                        buf.push_back(engulf());
                    }

                    if (buf == "exit") {
                        tokens.push_back({TokenType::exit, line_count});
                        buf.clear();
                    } else if (buf == "may") {
                        tokens.push_back({TokenType::may, line_count});
                        buf.clear();
                    } else if (buf == "if") {
                        tokens.push_back({TokenType::if_, line_count});
                        buf.clear();
                    } else if (buf == "elif") {
                        tokens.push_back({TokenType::elif, line_count});
                        buf.clear();
                    } else if (buf == "else") {
                        tokens.push_back({TokenType::else_, line_count});
                        buf.clear();
                    } else {
                        tokens.push_back({TokenType::ident, line_count,buf});
                        buf.clear();
                    }
                } else if (std::isdigit(peek().value())) {
                    buf.push_back(engulf());
                    while (peek().has_value() && std::isdigit(peek().value())) {
                        buf.push_back(engulf());
                    }

                    tokens.push_back({TokenType::int_lit, line_count,buf});
                    buf.clear();
                } else if (peek().value() == '-' && peek(1).has_value() && peek(1).value() == '-') {
                    engulf();
                    engulf();

                    while (peek().has_value() && peek().value() != '\n') {
                        engulf();
                    }
                } else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '*') {
                    engulf();
                    engulf();

                    while (peek().has_value()) {
                        if (peek().value() == '\n') {
                            line_count++;
                        }

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
                    tokens.push_back({TokenType::open_paren, line_count});
                } else if (peek().value() == ')') {
                    engulf();
                    tokens.push_back({TokenType::close_paren, line_count});
                }else if (peek().value() == ';') {
                    engulf();
                    tokens.push_back({TokenType::semi, line_count});
                } else if (peek().value() == '=') {
                    engulf();
                    tokens.push_back({TokenType::equal, line_count});
                } else if (peek().value() == '+') {
                    engulf();
                    tokens.push_back({TokenType::plus, line_count});
                } else if (peek().value() == '*') {
                    engulf();
                    tokens.push_back({TokenType::star, line_count});
                } else if (peek().value() == '/') {
                    engulf();
                    tokens.push_back({TokenType::fslash, line_count});
                } else if (peek().value() == '-') {
                    engulf();
                    tokens.push_back({TokenType::minus, line_count});
                } else if (peek().value() == '{') {
                    engulf();
                    tokens.push_back({TokenType::curly_open, line_count});
                } else if (peek().value() == '}') {
                    engulf();
                    tokens.push_back({TokenType::curly_close, line_count});
                } else if (peek().value() == '\n') {
                    engulf();
                    line_count++;
                }
                else if (std::isspace(peek().value())) {
                    engulf();
                } else {
                    std::cerr << "Invalid Token!" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }

            m_index = 0;
            return tokens;
        }

    private:
        const std::string m_src;
        size_t  m_index = 0;

         [[nodiscard]] std::optional<char> peek(const int offset = 0) const {
            if (m_index + offset >= m_src.length()) {
                return  {};
            }
            return m_src[m_index + offset];
         }

        char engulf() {
            return m_src[m_index++];
        }
};