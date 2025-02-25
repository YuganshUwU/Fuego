$$
\begin{align}
    [\text{Prog}] &\to [\text{Statement}]^*\\
    [\text{Statement}] &\to 
        \begin{cases}
            \text{exit([Expr]);}\\
            \text{may\space\ ident = [Expr];}\\
        \end{cases}\\
    [\text{Expr}] &\to 
        \begin{cases}
            \text{int\_lit}\\
            \text{ident}\\
            [\text{BinExpr}]
        \end{cases}\\
    [\text{BinExpr}] &\to 
        \begin{cases}
            [\text{Expr}] * \text{[Expr]}  & \text{prec = 1}\\
            [\text{Expr}] + \text{[Expr]}   & \text{prec = 0}\\
        \end{cases}\\
\end{align}
$$