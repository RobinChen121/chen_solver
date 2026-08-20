#ifndef CHEN_SOLVER_MODELING_H
#define CHEN_SOLVER_MODELING_H

#include <cmath>
#include <utility>
#include <vector>

#include "chen_solver/config.h"
#include "linear_term.h"

namespace chen_solver
{
    class Var
    {
        ChenInt col_{-1};

    public:
        Var() = default;
        explicit Var(const ChenInt col) : col_(col) {}

        [[nodiscard]] ChenInt col() const noexcept { return col_; }
        [[nodiscard]] bool isValid() const noexcept { return col_ >= 0; }
    };

    class LinExpr
    {
        std::vector<LinearTerm> terms_;
        double constant_{0.0};

    public:
        LinExpr() = default;
        LinExpr(const Var& var) : terms_{{.col = var.col(), .coef = 1.0}} {}

        [[nodiscard]] const std::vector<LinearTerm>& terms() const noexcept { return terms_; }
        [[nodiscard]] double constant() const noexcept { return constant_; }

        LinExpr& addTerm(const ChenInt col, const double coef)
        {
            if (std::abs(coef) > EPS)
            {
                terms_.push_back({.col = col, .coef = coef});
            }
            return *this;
        }

        LinExpr& addConstant(const double value)
        {
            constant_ += value;
            return *this;
        }

        LinExpr& operator+=(const LinExpr& other)
        {
            terms_.insert(terms_.end(), other.terms_.begin(), other.terms_.end());
            constant_ += other.constant_;
            return *this;
        }

        LinExpr& operator+=(const double value)
        {
            constant_ += value;
            return *this;
        }

        LinExpr& operator-=(const LinExpr& other)
        {
            for (const auto& term : other.terms_)
            {
                terms_.push_back({.col = term.col, .coef = -term.coef});
            }
            constant_ -= other.constant_;
            return *this;
        }

        LinExpr& operator-=(const double value)
        {
            constant_ -= value;
            return *this;
        }

        LinExpr& operator*=(const double scalar)
        {
            for (auto& term : terms_)
            {
                term.coef *= scalar;
            }
            constant_ *= scalar;
            return *this;
        }
    };

    class TempConstr
    {
        LinExpr expression_;
        double lb_{-INF};
        double ub_{INF};

    public:
        TempConstr(LinExpr expression, const double lb, const double ub)
            : expression_(std::move(expression)), lb_(lb), ub_(ub) {}

        [[nodiscard]] const LinExpr& expression() const noexcept { return expression_; }
        [[nodiscard]] double lb() const noexcept { return lb_; }
        [[nodiscard]] double ub() const noexcept { return ub_; }
    };

    inline LinExpr operator+(LinExpr lhs, const LinExpr& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    inline LinExpr operator+(LinExpr lhs, const double rhs)
    {
        lhs += rhs;
        return lhs;
    }

    inline LinExpr operator+(const double lhs, LinExpr rhs)
    {
        rhs += lhs;
        return rhs;
    }

    inline LinExpr operator-(LinExpr lhs, const LinExpr& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    inline LinExpr operator-(LinExpr lhs, const double rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    inline LinExpr operator-(const double lhs, const LinExpr& rhs)
    {
        LinExpr result;
        result += lhs;
        result -= rhs;
        return result;
    }

    inline LinExpr operator-(LinExpr expr)
    {
        expr *= -1.0;
        return expr;
    }

    inline LinExpr operator*(LinExpr expr, const double scalar)
    {
        expr *= scalar;
        return expr;
    }

    inline LinExpr operator*(const double scalar, LinExpr expr)
    {
        expr *= scalar;
        return expr;
    }

    inline LinExpr operator*(const double scalar, const Var& var)
    {
        LinExpr expr(var);
        expr *= scalar;
        return expr;
    }

    inline LinExpr operator*(const Var& var, const double scalar)
    {
        return scalar * var;
    }

    inline TempConstr operator<=(const LinExpr& lhs, const LinExpr& rhs)
    {
        return TempConstr(lhs - rhs, -INF, 0.0);
    }

    inline TempConstr operator<=(const LinExpr& lhs, const double rhs)
    {
        return lhs <= LinExpr().addConstant(rhs);
    }

    inline TempConstr operator<=(const double lhs, const LinExpr& rhs)
    {
        return TempConstr(rhs, lhs, INF);
    }

    inline TempConstr operator>=(const LinExpr& lhs, const LinExpr& rhs)
    {
        return TempConstr(lhs - rhs, 0.0, INF);
    }

    inline TempConstr operator>=(const LinExpr& lhs, const double rhs)
    {
        return lhs >= LinExpr().addConstant(rhs);
    }

    inline TempConstr operator>=(const double lhs, const LinExpr& rhs)
    {
        return TempConstr(rhs, -INF, lhs);
    }

    inline TempConstr operator==(const LinExpr& lhs, const LinExpr& rhs)
    {
        return TempConstr(lhs - rhs, 0.0, 0.0);
    }

    inline TempConstr operator==(const LinExpr& lhs, const double rhs)
    {
        return lhs == LinExpr().addConstant(rhs);
    }

    inline TempConstr operator==(const double lhs, const LinExpr& rhs)
    {
        return LinExpr().addConstant(lhs) == rhs;
    }

    inline TempConstr range(const double lb, const LinExpr& expression, const double ub)
    {
        return TempConstr(expression, lb, ub);
    }
} // namespace chen_solver

#endif // CHEN_SOLVER_MODELING_H
