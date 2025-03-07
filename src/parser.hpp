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

struct NodeTermParen {
    NodeExpr* expr;
};

struct BinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct BinExprMulti {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct BinExprSub {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct BinExprDiv {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    std::variant<BinExprAdd*, BinExprMulti*, BinExprSub*, BinExprDiv*> var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr {
    std::variant<NodeTerm* , NodeBinExpr*> var;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmtMay {
    Token ident;
    NodeExpr* expr{};
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
        } else if (const auto open_paren = try_engulf(TokenType::open_paren)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                std::cerr << "Expected an expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            try_engulf(TokenType::close_paren, "Expected `)`");
            auto term_paren = m_allocator.alloc<NodeTermParen>();
            term_paren->expr = expr.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_paren;
            return term;
        } else {
            return {};
        }
    }

    std::optional<NodeExpr*> parse_expr(int min_prec = 0) {

        std::optional<NodeTerm*> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }

        auto expr_lhs = m_allocator.alloc<NodeExpr>();
        expr_lhs->var = term_lhs.value();

        while (true) {
            std::optional<Token> curr_token = peek();
            std::optional<int> prec;

            if (curr_token.has_value()) {
                prec = bin_prec(curr_token->type);
                if (!prec.has_value() || prec < min_prec) {
                    break;
                }
            } else
                break;

            Token op = engulf();
            int next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(next_min_prec);

            if (!expr_rhs.has_value()) {
                std::cerr << "Unable to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            auto expr = m_allocator.alloc<NodeBinExpr>();
            auto expr_lhs_temp = m_allocator.alloc<NodeExpr>();

            if (op.type == TokenType::plus) {
                auto add = m_allocator.alloc<BinExprAdd>();
                expr_lhs_temp->var = expr_lhs->var;
                add->lhs = expr_lhs_temp;
                add->rhs = expr_rhs.value();
                expr->var = add;
            } else if (op.type == TokenType::star) {
                auto multi = m_allocator.alloc<BinExprMulti>();
                expr_lhs_temp->var = expr_lhs->var;
                multi->lhs = expr_lhs_temp;
                multi->rhs = expr_rhs.value();
                expr->var = multi;
            } else if (op.type == TokenType::sub) {
                auto sub = m_allocator.alloc<BinExprSub>();
                expr_lhs_temp->var = expr_lhs->var;
                sub->lhs = expr_lhs_temp;
                sub->rhs = expr_rhs.value();
                expr->var = sub;
            } else if (op.type == TokenType::div) {
                auto div = m_allocator.alloc<BinExprDiv>();
                expr_lhs_temp->var = expr_lhs->var;
                div->lhs = expr_lhs_temp;
                div->rhs = expr_rhs.value();
                expr->var = div;
            }

            expr_lhs->var = expr;
        }

        return expr_lhs;
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