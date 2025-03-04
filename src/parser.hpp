#pragma once
#include <variant>

#include "arena.hpp"
#include "tokenization.hpp"

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct BinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct BinExprMulti {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    BinExprAdd* add;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*> var;
};

struct NodeExpr {
    std::variant<NodeTerm* , NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtMay {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtMay*> var;
};

struct NodeProg {
    std::vector<NodeStmt> stmts;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)),
    m_allocator(1024 * 1024 * 4) {

    }

    std::optional<NodeTerm*> parse_term() {

        if (const auto int_lit = try_engulf(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        } else if (const auto ident = try_engulf(TokenType::ident)) {
            auto term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = ident.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        } else {
            return {};
        }
    }

    std::optional<NodeExpr*> parse_expr() {
        if (auto term = parse_term()) {
            if (try_engulf(TokenType::plus).has_value()) {
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();
                const auto bin_expr_add = m_allocator.alloc<BinExprAdd>();
                const auto lhs_expr = m_allocator.alloc<NodeExpr>();
                lhs_expr->var = term.value();
                bin_expr_add->lhs = lhs_expr;

                if (auto rhs = parse_expr()) {
                    bin_expr_add->rhs = rhs.value();
                    bin_expr->add = bin_expr_add;
                    auto expr = m_allocator.alloc<NodeExpr>();
                    expr->var = bin_expr;
                    return expr;

                } else {
                    std::cerr << "Expected expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                auto expr = m_allocator.alloc<NodeExpr>();
                expr->var = term.value();
                return expr;
            }
        } else {
            return {};
        }
    }

    std::optional<NodeStmt*> parse_stmt() {
        if (peek().has_value() && peek().value().type == TokenType::exit &&
            peek(1).has_value() && peek(1).value().type == TokenType::open_paren) {
            engulf();
            engulf();

            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (const auto node_expr = parse_expr()) {
                stmt_exit->expr = node_expr.value();
            } else {
                std::cerr << "Error parsing expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            try_engulf(TokenType::close_paren, "Expected ')'");
            try_engulf(TokenType::semi, "Expected ';'");


            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;

        } else if (peek().has_value() && peek().value().type == TokenType::may &&
            peek(1).has_value() && peek(1).value().type == TokenType::ident
            && peek(2).has_value() && peek(2).value().type == TokenType::equal) {
            engulf();
            auto stmt_may = m_allocator.alloc<NodeStmtMay>();
            stmt_may->ident = engulf();
            engulf();

            if (auto expr = parse_expr()) {
                stmt_may->expr = expr.value();
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            try_engulf(TokenType::semi, "Expected ';'");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_may;
            return stmt;
        }
        else
            return {};
    }

    std::optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(*stmt.value());
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

    inline Token try_engulf(const TokenType type, const std::string& err_msg) {
        if (peek().has_value() && peek().value().type == type) {
            return engulf();
        } else {
            std::cerr << err_msg << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    inline std::optional<Token> try_engulf(TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return engulf();
        } else {
            return {};
        }
    }

    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_allocator;
};