#pragma once
#include <unordered_map>

#include "parser.hpp"

class Generator {
public:
    inline explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {

    }

    void gen_term(const NodeTerm* term) {
        struct TermVisitor {
            Generator* gen;

            void operator()(const NodeTermIntLit* term_int_lit) const {
                gen->m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << "\n";
                gen->push("rax");
            }

            void operator()(const NodeTermIdent* term_ident) const {
                if (!gen->m_vars.contains(term_ident->ident.value.value())) {
                    std::cerr << "ERROR: Unknown identifier '" << term_ident->ident.value.value() << "'\n";
                    exit(EXIT_FAILURE);
                }

                const auto& var = gen->m_vars.at(term_ident->ident.value.value());
                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_loc - 1) * 8 << "]";
                gen->push(offset.str());
            }

            void operator()(const NodeTermParen* term_paren) const {
                gen->gen_expr(term_paren->expr);
            }
        };


        TermVisitor visitor({.gen = this});
        std::visit(visitor, term->var);
    }

    void gen_bin_expr(const NodeBinExpr* bin_expr) {
        struct BinExprVisitor {
            Generator* gen;

            void operator()(const BinExprAdd* expr_add) const {
                gen->gen_expr(expr_add->rhs);
                gen->gen_expr(expr_add->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    add rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const BinExprMulti* expr_multi) const {
                gen->gen_expr(expr_multi->rhs);
                gen->gen_expr(expr_multi->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    mul rbx\n";
                gen->push("rax");
            }

            void operator()(const BinExprSub* expr_sub) const {
                gen->gen_expr(expr_sub->rhs);
                gen->gen_expr(expr_sub->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    sub rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const BinExprDiv* expr_div) const {
                gen->gen_expr(expr_div->rhs);
                gen->gen_expr(expr_div->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    div rbx\n";
                gen->push("rax");
            }
        };

        BinExprVisitor visitor({.gen = this});
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const NodeExpr* expr) {
        struct ExprVisitor {

            Generator* gen;
            void operator()(const NodeTerm* term) const {
                gen->gen_term(term);
            }

            void operator()(const NodeBinExpr* bin_expr) const {
                gen->gen_bin_expr(bin_expr);
            }
        };

        ExprVisitor visitor{.gen = this};
        std::visit(visitor, expr->var);
    }

    void gen_stmt(const NodeStmt* stmt) {
        struct StmtVisitor {
            Generator* gen;

            void operator()(const NodeStmtExit* stmt_exit) const {
                gen->gen_expr(stmt_exit->expr);
                gen->m_output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }

            void operator()(const NodeStmtMay* stmt_may) const {
                if (gen->m_vars.contains(stmt_may->ident.value.value())) {
                    std::cerr <<"Identifier already used: " << stmt_may->ident.value.value() << "\n";
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.insert({stmt_may->ident.value.value(), Vars {.stack_loc = gen->m_stack_size}});
                gen->gen_expr(stmt_may->expr);
            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt->var);
    }

    [[nodiscard]] std::string gen_prog() {
        std::stringstream output;
        m_output << "global _start\n_start:\n";

        for (const NodeStmt& stmt : m_prog.stmts) {
            gen_stmt(&stmt);
        }

        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall\n";

        return m_output.str();
    }

private:

    void push(const std::string& reg) {
        m_output << "    push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const std::string& reg) {
    m_output << "    pop " << reg << "\n";
        m_stack_size--;
    }

    struct Vars {
        size_t stack_loc;
    };

    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Vars> m_vars {};

};