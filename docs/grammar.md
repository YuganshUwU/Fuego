$$
\begin{align}
    [\text{Prog}] &\to [\text{Statement}]^*\\
    [\text{Statement}] &\to 
        \begin{cases}
            \text{exit([Expr]);}\\
            \text{may\space\ ident = [Expr];}\\
            \text{ident} = [\text{Expr}];\\
            \text{[Scope]}\\
            \text{if ([Expr]) [Scope]} [\text{IfPred}]\\
            [\text{While}]\\
        \end{cases}\\
    [\text{Expr}] &\to 
        \begin{cases}
            [\text{Term}]\\
            [\text{BinExpr}]\\
        \end{cases}\\
    [\text{Scope}] &\to [\text{Statement*}]\\
    [\text{IfPred}] &\to
        \begin{cases}
            \text{elif} [\text{Expr}] [\text{Scope}] [\text{IfPred}]\\
            \text{else} [\text{Scope}]\\
            \epsilon\\
        \end{cases}\\
    [\text{While}] &\to \text{while } [\text{Expr}][\text{Scope}]\\
    [\text{BinExpr}] &\to 
        \begin{cases}
            [\text{Expr}] / [\text{Expr}] & \text{prec = 2}\\
            [\text{Expr}] * [\text{Expr}] & \text{prec = 2}\\
            [\text{Expr}] + [\text{Expr}] & \text{prec = 1}\\
            [\text{Expr}] - [\text{Expr}] & \text{prec = 1}\\
            [\text{Expr}] > [\text{Expr}] & \text{prec = 0}\\
            [\text{Expr}] >= [\text{Expr}] & \text{prec = 0}\\
            [\text{Expr}] < [\text{Expr}] & \text{prec = 0}\\
            [\text{Expr}] <= [\text{Expr}] & \text{prec = 0}\\
            [\text{Expr}] == [\text{Expr}] & \text{prec = 0}\\
            [\text{Expr}] != [\text{Expr}] & \text{prec = 0}\\
        \end{cases}\\
    [\text{Term}] &\to 
        \begin{cases}
            \text{int\_lit}\\
            \text{ident}\\
            ([\text{Expr}])\\
        \end{cases}
\end{align}
$$