/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 13:03
 * Description: 
 *
 */

#include "../../include/chen_solver/core/model.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <ranges>
#include <stdexcept>
#include <utility>

namespace chen_solver
{
    ChenInt ChenModel::addVariable(const double lb, const double ub,
                                   const VarType var_type, const std::string& name)
    {
        next_var_id = variables_.size();
        const std::string actual_name = name.empty() ? "x" + std::to_string(next_var_id) : name;

        if (name_to_varIndex.contains(actual_name))
            throw std::runtime_error("Duplicate variable name");

        const auto col = static_cast<ChenInt>(variables_.size());
        variables_.push_back(Variable{
            .id = next_var_id, .name = actual_name, .lb = lb, .ub = ub, .var_type = var_type
        });

        name_to_varIndex[variables_.back().name] = col;
        status_ = ModelStatus::Modified;
        return col;
    }

    Var ChenModel::addVar(const double lb, const double ub, const VarType var_type,
                          const std::string& name)
    {
        return Var(addVariable(lb, ub, var_type, name));
    }

    ChenInt ChenModel::addLinearConstraint(const std::vector<LinearTerm>& terms,
                                           const double lb,
                                           const double ub,
                                           const std::string& name)
    {
        next_con_id = constraints_.size();
        const std::string actual_name = name.empty() ? "c" + std::to_string(next_con_id) : name;

        if (name_to_conIndex.contains(actual_name))
        {
            throw std::runtime_error("Constraint already exists: " + actual_name);
        }

        LinearConstraint con;
        con.id = next_con_id++;
        con.name = actual_name;
        con.lb = lb;
        con.ub = ub;

        // 合并重复列，类似这种情况 2x_1 + 3x_1 + 4x_2 >= 10
        std::map<ChenInt, double> coeffs;
        for (const auto& [col, coef] : terms)
        {
            if (col < 0 || static_cast<std::size_t>(col) >= variables_.size())
                throw std::out_of_range("Invalid variable index in linear constraint");
            coeffs[col] += coef;
        }

        // 然后把 coeffs 传递到 con 里
        for (const auto& [col, coef] : coeffs)
        {
            if (std::abs(coef) > EPS)
            {
                con.lhs.push_back({.col = col, .coef = coef});
            }
        }

        constraints_.push_back(std::move(con));
        name_to_conIndex[actual_name] = static_cast<ChenInt>(constraints_.size() - 1);
        status_ = ModelStatus::Modified;
        return static_cast<ChenInt>(constraints_.size() - 1);
    }

    ChenInt ChenModel::addLineConstr(const std::vector<LinearTerm>& terms,
                                     const double lb,
                                     const double ub,
                                     const std::string& name)
    {
        return addLinearConstraint(terms, lb, ub, name);
    }

    ChenInt ChenModel::addLineConstr(const TempConstr& constraint, const std::string& name)
    {
        double lb = constraint.lb();
        double ub = constraint.ub();
        const double constant = constraint.expression().constant();

        if (lb > -INF / 2)
        {
            lb -= constant;
        }
        if (ub < INF / 2)
        {
            ub -= constant;
        }

        return addLineConstr(constraint.expression().terms(), lb, ub, name);
    }

    std::size_t ChenModel::numVariables() const noexcept
    {
        return variables_.size();
    }

    std::size_t ChenModel::numConstraints() const noexcept
    {
        return constraints_.size();
    }

    ModelStatus ChenModel::modelStatus() const noexcept
    {
        return status_;
    }

    void ChenModel::setObjective(const LinExpr& expression, const ObjSense sense)
    {
        objective_sense_ = sense;
        objective_offset = expression.constant();
        objective_coef.clear();

        std::map<ChenInt, double> merged_terms;
        for (const auto& [col, coef] : expression.terms())
        {
            if (col < 0 || static_cast<std::size_t>(col) >= variables_.size())
                throw std::out_of_range("Invalid variable index in objective");
            merged_terms[col] += coef;
        }

        for (const auto& [col, coef] : merged_terms)
        {
            if (std::abs(coef) > EPS)
            {
                objective_coef[col] = coef;
            }
        }

        status_ = ModelStatus::Modified;
    }

    [[nodiscard]] bool ChenModel::checkValid() const
    {
        if (std::ranges::any_of(variables_,
                                [](const auto& c) { return c.lb > c.ub; }))
            return false;
        if (std::ranges::any_of(constraints_,
                                [](const auto& c) { return c.lb > c.ub; }))
            return false;
        for (auto const& con : constraints_)
        {
            for (const auto& [col, coef] : con.lhs)
            {
                if (col < 0 || static_cast<std::size_t>(col) >= variables_.size())
                    return false;
            }
        }
        for (const auto& col : objective_coef | std::views::keys)
        {
            if (col < 0 || static_cast<std::size_t>(col) >= variables_.size())
                return false;
        }
        return true;
    }

    // 求解器中使用 nameToVarIndex
    ChenInt ChenModel::nameToVarIndex(const std::string& name) const
    {
        const auto it = name_to_varIndex.find(name);
        if (it == name_to_varIndex.end())
            throw std::runtime_error("Unknown variable: " + name);
        return it->second;
    }

    // LP/MPS Reader 中统一使用：findOrCreateVariable
    ChenInt ChenModel::findOrCreateVariable(const std::string& name)
    {
        const auto it = name_to_varIndex.find(name);
        if (it != name_to_varIndex.end())
            return it->second;

        addVariable(0.0, INF, VarType::Continuous, name);
        return static_cast<ChenInt>(variables_.size() - 1);
    }

    void ChenModel::print() const
    {
        printObjective();
        printConstraints();
        printBounds();
        printIntegers();
        printBinaries();
        std::cout << "End\n";
    }

    void ChenModel::printLinearExpression(const std::vector<LinearTerm>& terms) const
    {
        bool first = true;

        size_t counter = 0;
        for (const auto& [col, coef] : terms)
        {
            if (std::abs(coef) < 1e-12)
                continue;
            if (!first)
            {
                std::cout << (coef >= 0 ? " + " : " - ");
            }
            else if (coef < 0)
            {
                std::cout << "-";
            }

            if (const double abs_coef = std::abs(coef); std::abs(abs_coef - 1.0) > 1e-12)
                std::cout << abs_coef << " ";
            std::cout << variables_[col].name;
            counter++;
            if (counter % 3 == 0)
                std::cout << "\n      ";
            first = false;
        }

        if (first)
            std::cout << "0";
    }

    void ChenModel::printObjective() const
    {
        std::cout << (objective_sense_ == ObjSense::Minimize ? "Minimize\n" : "Maximize\n");
        std::cout << " obj: ";
        bool first = true;

        size_t counter = 0;
        for (auto const& [col, coef] : objective_coef)
        {
            if (std::abs(coef) < 1e-12)
                continue;

            if (!first)
            {
                std::cout << (coef >= 0 ? " + " : " - ");
            }
            else if (coef < 0)
            {
                std::cout << "-";
            }

            if (const double abs_coef = std::abs(coef); std::abs(abs_coef - 1.0) > 1e-12)
                std::cout << abs_coef << " ";
            std::cout << variables_[col].name;
            counter++;
            if (counter % 3 == 0)
                std::cout << "\n      ";
            first = false;
        }

        if (std::abs(objective_offset) > 1e-12)
        {
            if (!first)
            {
                std::cout << (objective_offset >= 0 ? " + " : " - ");
            }
            else if (objective_offset < 0)
            {
                std::cout << "-";
            }
            std::cout << std::abs(objective_offset);
            first = false;
        }

        if (first) // 目标函数全部为0
            std::cout << "0";
        std::cout << "\n\n";
    }

    void ChenModel::printConstraints() const
    {
        std::cout << "Subject To\n";
        for (auto& con : constraints_)
        {
            std::cout << " " << (con.name.empty() ? "c" : con.name) << ": ";
            printLinearExpression(con.lhs);

            if (std::abs(con.lb - con.ub) < 1e-12)
            {
                std::cout << " = " << con.ub;
            }
            else if (con.lb <= -INF / 2)
            {
                std::cout << " <= " << con.ub;
            }
            else if (con.ub >= INF / 2)
            {
                std::cout << " >= " << con.lb;
            }
            else
            {
                // 处理约束条件在可能的 ranges 的情况
                std::cout << " in [" << con.lb << ", " << con.ub << "]";
            }

            std::cout << "\n";
        }

        std::cout << "\n";
    }

    void ChenModel::printBounds() const
    {
        std::cout << "Bounds\n";
        for (const auto& var : variables_)
        {
            if (var.var_type == VarType::Binary)
            {
                continue;
            }
            if (var.lb <= -INF / 2 && var.ub >= INF / 2)
            {
                std::cout << " " << var.name << " free\n";
            }
            else if (std::abs(var.lb - var.ub) < 1e-12)
            {
                std::cout << " " << var.name << " = " << var.lb << "\n";
            }
            else if (var.lb > -INF / 2 && var.ub < INF / 2)
            {
                std::cout << " " << var.lb << " <= " << var.name << " <= " << var.ub << "\n";
            }
            else if (var.lb > -INF / 2)
            {
                std::cout << " " << var.name << " >= " << var.lb << "\n";
            }
            else if (var.ub < INF / 2)
            {
                std::cout << " " << var.name << " <= " << var.ub << "\n";
            }
        }

        std::cout << "\n";
    }

    void ChenModel::printIntegers() const
    {
        bool has_integer = false;
        for (const auto& var : variables_)
            if (var.var_type == VarType::Integer)
                has_integer = true;

        if (!has_integer)
            return;

        std::cout << "Generals\n";
        for (const auto& var : variables_)
            if (var.var_type == VarType::Integer)
                std::cout << " " << var.name << "\n";

        std::cout << "\n";
    }

    void ChenModel::printBinaries() const
    {
        bool has_binary = false;
        for (const auto& var : variables_)
            if (var.var_type == VarType::Binary)
                has_binary = true;

        if (!has_binary)
            return;

        std::cout << "Binaries\n";
        for (const auto& var : variables_)
            if (var.var_type == VarType::Binary)
                std::cout << " " << var.name << "\n";

        std::cout << "\n";
    }

    void ChenModel::optimize()
    {
        return;
    }
} // namespace chen_solver
