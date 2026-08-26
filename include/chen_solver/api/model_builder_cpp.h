/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/25, 17:01
 * Description: provide a modeling interface of CPP for users to build models in a more convenient way,
 * similar to Gurobi.
 *
 */


#ifndef CHEN_SOLVER_MODELING_H
#define CHEN_SOLVER_MODELING_H

#include <cmath>
#include <utility>
#include <vector>

#include "chen_solver/config.h"
#include "chen_solver/core/linear_term.h"

class Var {
    ChenInt col_{-1};

public:
    Var() = default;
    explicit Var(const ChenInt col) : col_(col) {
    }

    // noexcept 表示这个函数不会抛出异常，[[nodiscard]] 表示编译器会警告如果返回值被忽略
    [[nodiscard]] ChenInt col() const noexcept { return col_; }

    [[nodiscard]] bool isValid() const noexcept { return col_ >= 0; }
};

class LinExpr {
    std::vector<LinearTerm> terms_;
    double constant_{0.0};

public:
    LinExpr() = default;
    LinExpr(const Var &var) : terms_{{.col = var.col(), .coef = 1.0}} {
    }

    [[nodiscard]] const std::vector<LinearTerm> &terms() const noexcept { return terms_; }

    [[nodiscard]] double constant() const noexcept { return constant_; }

    LinExpr &addTerm(const double coef, const Var &var) {
        if (std::abs(coef) <= EPS) {
            return *this;
        }

        const auto col = var.col();
        for (auto it = terms_.begin(); it != terms_.end(); ++it) {
            if (it->col == col) {
                it->coef += coef;
                if (std::abs(it->coef) <= EPS) {
                    terms_.erase(it);
                }
                return *this;
            }
        }

        terms_.push_back({.col = col, .coef = coef});
        return *this;
    }

    LinExpr &addConstant(const double value) {
        constant_ += value;
        return *this;
    }

    // C++ 中重载运算法 +=, -=, *=：修改自身，返回自身的类引用
    // 而重载运算符 +， -，则返回一个新的对象
    LinExpr &operator+=(const LinExpr &other) {
        for (const auto &[col, coef]: other.terms_) {
            addTerm(coef, Var(col));
        }
        constant_ += other.constant_;
        return *this;
    }

    LinExpr &operator+=(const double value) {
        constant_ += value;
        return *this;
    }

    LinExpr &operator-=(const LinExpr &other) {
        for (const auto &[col, coef]: other.terms_) {
            addTerm(-coef, Var(col));
        }
        constant_ -= other.constant_;
        return *this;
    }

    LinExpr &operator-=(const double value) {
        constant_ -= value;
        return *this;
    }

    LinExpr &operator*=(const double scalar) {
        for (auto &[col, coef]: terms_) {
            coef *= scalar;
        }
        constant_ *= scalar;
        return *this;
    }
};

class TempConstr {
    LinExpr expression_;
    double lb_{-INF};
    double ub_{INF};

public:
    // 使用 std::move 来避免不必要的拷贝，提高性能
    // 使用 LinExpr &expression 不妥，会导致不能接临时对象、接口限制大
    // 若使用 const LinExpr &expression，可以接收临时对象，但初始化成员时通常只能拷贝，无法移动
    // 只读、不保存、不转移所有权 → 用 const T&
    // 要保存一份副本（成员/容器） → 常用 T（按值）+ std::move
    TempConstr(LinExpr expression, const double lb, const double ub)
        : expression_(std::move(expression)), lb_(lb), ub_(ub) {
    }

    [[nodiscard]] const LinExpr &expression() const noexcept { return expression_; }

    [[nodiscard]] double lb() const noexcept { return lb_; }

    [[nodiscard]] double ub() const noexcept { return ub_; }
};

// 重载运算符，使得用户可以使用类似数学表达式的方式来构建线性表达式和约束条件
// 第一个参数没有用&，是因为相加时左边不变，需要创建一个新的LinExpr对象，不能直接修改原来的对象。
inline LinExpr operator+(LinExpr lhs, const LinExpr &rhs) {
    lhs += rhs;
    return lhs;
}

inline LinExpr operator+(LinExpr lhs, const Var &rhs) {
    lhs += LinExpr(rhs);
    return lhs;
}

// Var 不能修改，所以这里可以使用引用结合 const 传递
inline LinExpr operator+(const Var &lhs, const LinExpr &rhs) {
    return LinExpr(lhs) + rhs;
}

inline LinExpr operator+(const Var &lhs, const Var &rhs) {
    return LinExpr(lhs) + LinExpr(rhs);
}

inline LinExpr operator+(LinExpr lhs, const double rhs) {
    lhs += rhs;
    return lhs;
}

inline LinExpr operator+(const Var &lhs, const double rhs) {
    return LinExpr(lhs) + rhs;
}

inline LinExpr operator+(const double lhs, LinExpr rhs) {
    rhs += lhs; // 这与对应的那个 operator- 有点不一样，因为减法不能交换前后顺序
    return rhs;
}

inline LinExpr operator+(const double lhs, const Var &rhs) {
    return lhs + LinExpr(rhs);
}

inline LinExpr operator-(LinExpr lhs, const LinExpr &rhs) {
    lhs -= rhs;
    return lhs;
}

inline LinExpr operator-(LinExpr lhs, const Var &rhs) {
    lhs -= LinExpr(rhs);
    return lhs;
}

inline LinExpr operator-(const Var &lhs, const LinExpr &rhs) {
    return LinExpr(lhs) - rhs;
}

inline LinExpr operator-(const Var &lhs, const Var &rhs) {
    return LinExpr(lhs) - LinExpr(rhs);
}

inline LinExpr operator-(LinExpr lhs, const double rhs) {
    lhs -= rhs;
    return lhs;
}

inline LinExpr operator-(const Var &lhs, const double rhs) {
    return LinExpr(lhs) - rhs;
}

inline LinExpr operator-(const double lhs, const LinExpr &rhs) {
    LinExpr result; // 因为 double 没有 - 函数，因此首先将这个double 转化成 LinExpr 对象，然后再进行减法运算
    result += lhs;
    result -= rhs;
    return result;
}

inline LinExpr operator-(const double lhs, const Var &rhs) {
    return lhs - LinExpr(rhs);
}

inline LinExpr operator-(LinExpr expr) {
    expr *= -1.0;
    return expr;
}

inline LinExpr operator*(LinExpr expr, const double scalar) {
    expr *= scalar;
    return expr;
}

inline LinExpr operator*(const double scalar, LinExpr expr) {
    expr *= scalar;
    return expr;
}

inline LinExpr operator*(const double scalar, const Var &var) {
    LinExpr expr(var);
    expr *= scalar;
    return expr;
}

inline LinExpr operator*(const Var &var, const double scalar) {
    return scalar * var;
}

inline TempConstr operator<=(const LinExpr &lhs, const LinExpr &rhs) {
    return TempConstr(lhs - rhs, -INF, 0.0);
}

inline TempConstr operator<=(const LinExpr &lhs, const double rhs) {
    return lhs <= LinExpr().addConstant(rhs);
}

inline TempConstr operator<=(const double lhs, const LinExpr &rhs) {
    return TempConstr(rhs, lhs, INF);
}

inline TempConstr operator>=(const LinExpr &lhs, const LinExpr &rhs) {
    return TempConstr(lhs - rhs, 0.0, INF);
}

inline TempConstr operator>=(const LinExpr &lhs, const double rhs) {
    return lhs >= LinExpr().addConstant(rhs);
}

inline TempConstr operator>=(const double lhs, const LinExpr &rhs) {
    return TempConstr(rhs, -INF, lhs);
}

inline TempConstr operator==(const LinExpr &lhs, const LinExpr &rhs) {
    return TempConstr(lhs - rhs, 0.0, 0.0);
}

inline TempConstr operator==(const LinExpr &lhs, const double rhs) {
    return lhs == LinExpr().addConstant(rhs);
}

inline TempConstr operator==(const double lhs, const LinExpr &rhs) {
    return LinExpr().addConstant(lhs) == rhs;
}

inline TempConstr range(const double lb, const LinExpr &expression, const double ub) {
    return TempConstr(expression, lb, ub);
}
#endif // CHEN_SOLVER_MODELING_H
