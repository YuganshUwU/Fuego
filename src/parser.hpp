#pragma once
#include <variant>

#include "tokenization.hpp"

struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprIdent {
    Token ident;
};

struct NodeExpr {
    std::variant<NodeExprIntLit, NodeExprIdent> var;
};

struct NodeStmtExit {
    NodeExpr expr;
};

struct NodeStmtMay {
    Token ident;
    NodeExpr expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit, NodeStmtMay> var;
};

struct NodeProg {
    std::vector<NodeStmt> stmts;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {

    }

    std::optional<NodeExpr> parse_expr() {
        if (peek().has_value() && peek().value().type == TokenType::int_lit) {
            return NodeExpr {.var = NodeExprIntLit {.int_lit = engulf()}};
        } else if (peek().has_value() && peek().value().type == TokenType::ident) {
            return NodeExpr {.var = NodeExprIdent {.ident = engulf()}};
        }else {
            return {};
        }
    }

    std::optional<NodeStmt> parse_stmt() {
        if (peek().has_value() && peek().value().type == TokenType::exit &&
            peek(1).has_value() && peek(1).value().type == TokenType::open_paren) {
            engulf();
            engulf();

            NodeStmtExit stmt_exit;
            if (const auto node_expr = parse_expr()) {
                stmt_exit = {.expr = node_expr.value()};
            } else {
                std::cerr << "Error parsing expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (peek().has_value() && peek().value().type == TokenType::close_paren) {
                engulf();
            } else {
                std::cerr << "Expected ')'" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (peek().has_value() && peek().value().type == TokenType::semi) {
                engulf();
            } else {
                std::cerr << "Expected ';'" << std::endl;
                exit(EXIT_FAILURE);
            }

            return NodeStmt {.var = stmt_exit};

        } else if (peek().has_value() && peek().value().type == TokenType::may &&
            peek(1).has_value() && peek(1).value().type == TokenType::ident
            && peek(2).has_value() && peek(2).value().type == TokenType::equal) {
            engulf();
            auto stmt_may = NodeStmtMay{.ident = engulf()};
            engulf();

            if (auto expr = parse_expr()) {
                stmt_may.expr = expr.value();
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (peek().has_value() && peek().value().type == TokenType::semi) {
                engulf();
            } else {
                std::cerr << "Expected ';'" << std::endl;
                exit(EXIT_FAILURE);
            }

            return NodeStmt{.var = stmt_may};
        }
        else
            return {};
    }

    std::optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            } else {
                std::cerr << "Invalid statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        return prog;
    }

private:

    [[nodiscard]] inline std::optional<Token> peek(int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        } else
            return m_tokens.at(m_index + offset);
    }

    inline Token engulf() {
        return m_tokens.at(m_index++);
    }

    const std::vector<Token> m_tokens;
    size_t m_index = 0;
};