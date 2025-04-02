#pragma once

#include "parser.hpp"
#include <algorithm>
#include <assert.h>

class Generator {
public:
    explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {
    }

    void gen_term(const NodeTerm *term) {
        struct TermVisitor {
            Generator &gen;

            void operator()(const NodeTermIntLit *term_int_lit) const {
                gen.m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << "\n";
                gen.push("rax");
            }

            void operator()(const NodeTermIdent *term_ident) const {
                const auto it = std::find_if(gen.m_vars.cbegin(), gen.m_vars.cend(),
                                             [&](const Vars &var) {
                                                 return var.name == term_ident->ident.value.value();
                                             });
                if (it == gen.m_vars.cend()) {
                    std::cerr << "ERROR: Unknown identifier '" << term_ident->ident.value.value() << "'\n";
                    exit(EXIT_FAILURE);
                }

                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen.m_stack_size - it->stack_loc - 1) * 8 << "]";
                gen.push(offset.str());
            }

            void operator()(const NodeTermParen *term_paren) const {
                gen.gen_expr(term_paren->expr);
            }
        };


        TermVisitor visitor({.gen = *this});
        std::visit(visitor, term->var);
    }

    void gen_bin_expr(const NodeBinExpr *bin_expr) {
        struct BinExprVisitor {
            Generator &gen;

            void operator()(const BinExprAdd *expr_add) const {
                gen.gen_expr(expr_add->rhs);
                gen.gen_expr(expr_add->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "    add rax, rbx\n";
                gen.push("rax");
            }

            void operator()(const BinExprMulti *expr_multi) const {
                gen.gen_expr(expr_multi->rhs);
                gen.gen_expr(expr_multi->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "    mul rbx\n";
                gen.push("rax");
            }

            void operator()(const BinExprSub *expr_sub) const {
                gen.gen_expr(expr_sub->rhs);
                gen.gen_expr(expr_sub->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "    sub rax, rbx\n";
                gen.push("rax");
            }

            void operator()(const BinExprDiv *expr_div) const {
                gen.gen_expr(expr_div->rhs);
                gen.gen_expr(expr_div->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                gen.m_output << "    div rbx\n";
                gen.push("rax");
            }

            void operator()(const BinExprGreater *expr_greater) const {
                gen.gen_expr(expr_greater->rhs);
                gen.gen_expr(expr_greater->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                const std::string label = gen.create_label();
                const std::string newLabel = gen.create_label();
                gen.m_output << "    cmp rax, rbx\n";
                gen.m_output << "    jg " << label << "\n";
                gen.m_output << "    mov rax, 0\n";
                gen.m_output << "    jmp " << newLabel << "\n\n";
                gen.m_output << label << ":\n";
                gen.m_output << "    mov rax, 1\n";
                gen.m_output << newLabel << ":\n";
                gen.push("rax");
            }

            void operator()(const BinExprLess *less) const {
                gen.gen_expr(less->rhs);
                gen.gen_expr(less->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                const std::string label = gen.create_label();
                const std::string newLabel = gen.create_label();

                gen.m_output << "    cmp rax, rbx\n";
                gen.m_output << "    jl " << label << "\n";
                gen.m_output << "    mov rax, 0\n";
                gen.m_output << "    jmp " << newLabel << "\n\n";
                gen.m_output << label << ":\n";
                gen.m_output << "    mov rax, 1\n";
                gen.m_output << newLabel << ":\n";
                gen.push("rax");
            }

            void operator()(const BinExprEqual *expr_equal) const {
                gen.gen_expr(expr_equal->rhs);
                gen.gen_expr(expr_equal->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                const std::string label = gen.create_label();
                const std::string newLabel = gen.create_label();

                gen.m_output << "    cmp rax, rbx\n";
                gen.m_output << "    je " << label << "\n";
                gen.m_output << "    mov rax, 0\n";
                gen.m_output << "    jmp " << newLabel << "\n\n";
                gen.m_output << label << ":\n";
                gen.m_output << "    mov rax, 1\n";
                gen.m_output << newLabel << ":\n";
                gen.push("rax");
            }

            void operator()(const BinExprGreaterEqual *expr_greater_equal) const {
                gen.gen_expr(expr_greater_equal->rhs);
                gen.gen_expr(expr_greater_equal->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                const std::string label = gen.create_label();
                const std::string newLabel = gen.create_label();
                gen.m_output << "    cmp rax, rbx\n";
                gen.m_output << "    jge " << label << "\n";
                gen.m_output << "    mov rax, 0\n";
                gen.m_output << "    jmp " << newLabel << "\n\n";
                gen.m_output << label << ":\n";
                gen.m_output << "    mov rax, 1\n";
                gen.m_output << newLabel << ":\n";
                gen.push("rax");
            }

            void operator()(const BinExprLessEqual *expr_less_equal) const {
                gen.gen_expr(expr_less_equal->rhs);
                gen.gen_expr(expr_less_equal->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                const std::string label = gen.create_label();
                const std::string newLabel = gen.create_label();

                gen.m_output << "    cmp rax, rbx\n";
                gen.m_output << "    jle " << label << "\n";
                gen.m_output << "    mov rax, 0\n";
                gen.m_output << "    jmp " << newLabel << "\n\n";
                gen.m_output << label << ":\n";
                gen.m_output << "    mov rax, 1\n";
                gen.m_output << newLabel << ":\n";
                gen.push("rax");
            }

            void operator()(const BinExprNotEqual *expr_not_equal) const {
                gen.gen_expr(expr_not_equal->rhs);
                gen.gen_expr(expr_not_equal->lhs);
                gen.pop("rax");
                gen.pop("rbx");
                const std::string label = gen.create_label();
                const std::string newLabel = gen.create_label();

                gen.m_output << "    cmp rax, rbx\n";
                gen.m_output << "    jne " << label << "\n";
                gen.m_output << "    mov rax, 0\n";
                gen.m_output << "    jmp " << newLabel << "\n\n";
                gen.m_output << label << ":\n";
                gen.m_output << "    mov rax, 1\n";
                gen.m_output << newLabel << ":\n";
                gen.push("rax");
            }
        };

        BinExprVisitor visitor({.gen = *this});
        std::visit(visitor, bin_expr->var);
    }

    void gen_scope(const NodeStmtScope *scope) {
        begin_scopes();
        for (const NodeStmt *stmt: scope->stmts) {
            gen_stmt(stmt);
        }
        end_scopes();
    }

    void gen_expr(const NodeExpr *expr) {
        struct ExprVisitor {
            Generator &gen;

            void operator()(const NodeTerm *term) const {
                gen.gen_term(term);
            }

            void operator()(const NodeBinExpr *bin_expr) const {
                gen.gen_bin_expr(bin_expr);
            }
        };

        ExprVisitor visitor{.gen = *this};
        std::visit(visitor, expr->var);
    }

    void gen_if_pred(const NodeStmtIfPred *pred, const std::string &end_label) {
        struct PredVisitor {
            Generator &gen;
            const std::string &end_label;

            void operator()(const NodeStmtIfPredElif *elif) const {
                gen.gen_expr(elif->expr);
                gen.pop("rax");
                const std::string label = gen.create_label();
                gen.m_output << "    test rax, rax\n";
                gen.m_output << "    jz " << label << "\n";
                gen.gen_scope(elif->scope);
                gen.m_output << "    jmp " << end_label << "\n";
                gen.m_output << label << ":\n";
                if (elif->pred.has_value()) {
                    gen.gen_if_pred(elif->pred.value(), end_label);
                }
            }

            void operator()(const NodeStmtIfPredElse *else_) const {
                gen.gen_scope(else_->scope);
            }
        };

        PredVisitor visitor{.gen = *this, .end_label = end_label};
        std::visit(visitor, pred->var);
    }

    void gen_stmt(const NodeStmt *stmt) {
        struct StmtVisitor {
            Generator &gen;

            void operator()(const NodeStmtExit *stmt_exit) const {
                gen.gen_expr(stmt_exit->expr);
                gen.m_output << "    mov rax, 60\n";
                gen.pop("rdi");
                gen.m_output << "    syscall\n";
            }

            void operator()(const NodeStmtMay *stmt_may) const {
                const auto it = std::find_if(gen.m_vars.cbegin(), gen.m_vars.cend(),
                            [&](const Vars &var) { return var.name == stmt_may->ident.value.value(); });

                if (it != gen.m_vars.cend()) {
                    std::cerr << "Identifier already used: " << stmt_may->ident.value.value() << "\n";
                    exit(EXIT_FAILURE);
                }
                gen.m_vars.push_back({.name = stmt_may->ident.value.value(), .stack_loc = gen.m_stack_size});
                gen.gen_expr(stmt_may->expr);
            }

            void operator()(const NodeStmtAssign *stmt_assign) const {
                const auto it = std::find_if(gen.m_vars.cbegin(), gen.m_vars.cend(),
                                [&](const Vars &var) {return var.name == stmt_assign->ident.value.value();});

                if (it == gen.m_vars.cend()) {
                    std::cerr << "Undeclared Identifier" << stmt_assign->ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }

                gen.gen_expr(stmt_assign->expr);
                gen.pop("rax");
                gen.m_output << "    mov [rsp + " << (gen.m_stack_size - it->stack_loc - 1) * 8 << "], rax\n";
            }

            void operator()(const NodeStmtScope *stmt_scope) const {
                gen.gen_scope(stmt_scope);
            }

            void operator()(const NodeStmtIf *stmt_if) const {
                gen.gen_expr(stmt_if->expr);
                gen.pop("rax");
                const std::string label = gen.create_label();
                gen.m_output << "    test rax, rax\n";
                gen.m_output << "    jz " << label << "\n";
                gen.gen_scope(stmt_if->scope);
                if (stmt_if->pred.has_value()) {
                    const std::string end_label = gen.create_label();
                    gen.m_output << "    jmp " << end_label << "\n";
                    gen.m_output << label << ":\n";
                    gen.gen_if_pred(stmt_if->pred.value(), end_label);
                    gen.m_output << end_label << ":\n";
                } else {
                    gen.m_output << label << ":\n";
                }
            }

            void operator()(const NodeStmtWhile *stmt_while) const {
                const std::string begin_label = gen.create_label();
                const std::string tle_label = gen.create_label();
                const std::string end_label = gen.create_label();
                gen.m_output << "    mov rcx, 1000000000\n";

                gen.m_output << begin_label << ":\n";
                gen.gen_expr(stmt_while->expr);
                gen.pop("rax");
                gen.m_output << "    test rax, rax\n";
                gen.m_output << "    jz " << end_label << "\n";
                gen.m_output << "    dec rcx\n";
                gen.m_output << "    cmp rcx, $0\n";
                gen.m_output << "    jle " << tle_label << "\n";

                gen.gen_scope(stmt_while->scope);
                gen.m_output << "    test rax, rax\n";
                gen.m_output << "    jnz " << begin_label << "\n";

                gen.m_output << tle_label << ":\n";
                gen.m_output << "    mov rax, 1\n";
                gen.m_output << "    mov rdi, 1\n";
                gen.m_output << "    mov rsi, msg\n";
                gen.m_output << "    mov rdx, len\n";
                gen.m_output << "    syscall\n";

                gen.m_output << "    mov rax, 60\n";
                gen.m_output << "    mov rdi, 0\n";
                gen.m_output << "    syscall\n";

                gen.m_output << end_label << ":\n";
            }

            void operator()(const NodeStmtFor* for_stmt) const {
                const std::string start_label = gen.create_label();  
                const std::string end_label = gen.create_label();    
                const std::string increment_label = gen.create_label();
            
                gen.gen_stmt(for_stmt->init);
                gen.m_output << "    jmp " << start_label << "\n";  
                
                gen.m_output << start_label << ":\n";
                gen.gen_expr(for_stmt->cond);
                gen.pop("rax");  
                gen.m_output << "    test rax, rax\n";  
                gen.m_output << "    jz " << end_label << "\n";  
            
                gen.gen_scope(for_stmt->scope);
                    
                gen.m_output << increment_label << ":\n";
                gen.gen_stmt(for_stmt->iter);  
                gen.m_output << "    jmp " << start_label << "\n";  
            
                gen.m_output << end_label << ":\n";
            }            
        };

        StmtVisitor visitor{.gen = *this};
        std::visit(visitor, stmt->var);
    }

    [[nodiscard]] std::string gen_prog() {
        std::stringstream output;
        m_output << "section .data\n";
        m_output << "    msg db \"Oops! Time Limit Exceeded, check your logic\", 0xa\n";
        m_output << "    len EQU $ - msg\n";

        m_output << "\nsection .text\n";
        m_output << "    global _start\n_start:\n";

        for (const NodeStmt &stmt: m_prog.stmts) {
            gen_stmt(&stmt);
        }

        return m_output.str();
    }

private:
    void push(const std::string &reg) {
        m_output << "    push " << reg << "\n";
        m_stack_size++;
    }

    void pop(const std::string &reg) {
        m_output << "    pop " << reg << "\n";
        m_stack_size--;
    }

    void begin_scopes() {
        m_scopes.push_back(m_vars.size());
    }

    void end_scopes() {
        const size_t pop_count = m_vars.size() - m_scopes.back();
        m_output << "    add rsp, " << pop_count * 8 << "\n";
        m_stack_size -= pop_count;

        for (int i = 0; i < pop_count; i++) {
            m_vars.pop_back();
        }

        m_scopes.pop_back();
    }

    struct Vars {
        std::string name;
        size_t stack_loc;
    };

    std::string create_label() {
        return "label" + std::to_string(m_label_count++);
    }

    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::vector<Vars> m_vars{};
    std::vector<size_t> m_scopes{};
    int m_label_count = 0;
};
