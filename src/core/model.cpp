#include "chen_solver/model.h"


#include <iostream>
#include <utility>

namespace chen_solver {
    void Model::addVariable(const std::string &name, const double lb, const double ub,
                            const VarType var_type) {
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
    };

    void Model::addLinearConstraint(const std::string &name, const std::vector<LinearTerm> &terms,
                                    const double lb,
                                    const double ub) {
        next_con_id = constraints_.size();
        const std::string actual_name = name.empty() ? "c" + std::to_string(next_con_id) : name;

        if (name_to_conIndex.contains(actual_name)) {
            throw std::runtime_error("Constraint already exists: " + actual_name);
        }

        LinearConstraint con;
        con.id = next_con_id++;
        con.name = actual_name;
        con.lb = lb;
        con.ub = ub;

        // 合并重复列，类似这种情况 2x_1 + 3x_1 + 4x_2 >= 10
        std::map<ChenInt, double> coeffs;
        for (const auto &[col, coef]: terms) {
            coeffs[col] += coef;
        }

        // 然后把 coeffs 传递到 con 里
        for (const auto &[col, coef]: coeffs) {
            if (std::abs(coef) > EPS) {
                con.lhs.push_back({.col = col, .coef = coef});
            }
        }

        constraints_.push_back(std::move(con));
        name_to_conIndex[name] = static_cast<ChenInt>(constraints_.size() - 1);
        status_ = ModelStatus::Modified;
    }

    std::size_t Model::numVariables() const noexcept {
        return variables_.size();
    }

    std::size_t Model::numConstraints() const noexcept {
        return constraints_.size();
    }

    ModelStatus Model::modelStatus() const noexcept {
        return status_;
    }

    [[nodiscard]] bool Model::checkValid() const {
        if (std::ranges::any_of(variables_, [](const auto &c) { return c.lb > c.ub; }))
            return false;
        if (std::ranges::all_of(constraints_, [](const auto &c) { return c.lb < c.ub; }))
            return false;
        for (auto const &con: constraints_) {
            for (const auto &[col, coef]: con.lhs) {
                if (col < 0 || col >= variables_.size())
                    return false;
            }
        }
        return true;
    }

    // 求解器中使用 nameToVarIndex
    ChenInt Model::nameToVarIndex(const std::string &name) const {
        const auto it = name_to_varIndex.find(name);
        if (it == name_to_varIndex.end())
            throw std::runtime_error("Unknown variable: " + name);
        return it->second;
    }

    // LP/MPS Reader 中统一使用：findOrCreateVariable
    ChenInt Model::findOrCreateVariable(const std::string &name) {
        const auto it = name_to_varIndex.find(name);
        if (it != name_to_varIndex.end())
            return it->second;

        addVariable(name);
        return static_cast<ChenInt>(variables_.size() - 1);
    }

    void Model::printLinearExpression(const std::vector<LinearTerm> &terms) const {
        bool first = true;

        size_t counter = 0;
        for (const auto &[col, coef]: terms) {
            if (std::abs(coef) < 1e-12)
                continue;
            if (!first) {
                std::cout << (coef >= 0 ? " + " : " - ");
            } else if (coef < 0) {
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

    void Model::printObjective() const {
        std::cout << (objective_sense_ == ObjSense::Minimize ? "Minimize\n" : "Maximize\n");
        std::cout << " obj: ";
        bool first = true;

        size_t counter = 0;
        for (auto const &[col, coef]: objective_coef) {
            if (std::abs(coef) < 1e-12)
                continue;

            if (!first) {
                std::cout << (coef >= 0 ? " + " : " - ");
            } else if (coef < 0) {
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

        if (first) // 目标函数全部为0
            std::cout << "0";
        std::cout << "\n\n";
    }

    void Model::printConstraints() const {
        std::cout << "Subject To\n";
        for (auto &con: constraints_) {
            std::cout << " " << (con.name.empty() ? "c" : con.name) << ": ";
            printLinearExpression(con.lhs);

            if (std::abs(con.lb - con.ub) < 1e-12) {
                std::cout << " = " << con.ub;
            } else if (con.lb <= -INF / 2) {
                std::cout << " <= " << con.ub;
            } else if (con.ub >= INF / 2) {
                std::cout << " >= " << con.lb;
            } else {
                // 处理约束条件在可能的 ranges 的情况
                std::cout << " in [" << con.lb << ", " << con.ub << "]";
            }

            std::cout << "\n";
        }

        std::cout << "\n";
    }

    void Model::printBounds() const {
        std::cout << "Bounds\n";
        for (const auto &var: variables_) {
            if (var.var_type == VarType::Binary) {
                continue;
            }
            if (var.lb <= -INF / 2 && var.ub >= INF / 2) {
                std::cout << " " << var.name << " free\n";
            } else if (std::abs(var.lb - var.ub) < 1e-12) {
                std::cout << " " << var.name << " = " << var.lb << "\n";
            } else if (var.lb > -INF / 2 && var.ub < INF / 2) {
                std::cout << " " << var.lb << " <= " << var.name << " <= " << var.ub << "\n";
            } else if (var.lb > -INF / 2) {
                std::cout << " " << var.name << " >= " << var.lb << "\n";
            } else if (var.ub < INF / 2) {
                std::cout << " " << var.name << " <= " << var.ub << "\n";
            }
        }

        std::cout << "\n";
    }

    void Model::printIntegers() const {
        bool has_integer = false;
        for (const auto &var: variables_)
            if (var.var_type == VarType::Integer)
                has_integer = true;

        if (!has_integer)
            return;

        std::cout << "Generals\n";
        for (const auto &var: variables_)
            if (var.var_type == VarType::Integer)
                std::cout << " " << var.name << "\n";

        std::cout << "\n";
    }

    void Model::printBinaries() const {
        bool has_binary = false;
        for (const auto &var: variables_)
            if (var.var_type == VarType::Binary)
                has_binary = true;

        if (!has_binary)
            return;

        std::cout << "Binaries\n";
        for (const auto &var: variables_)
            if (var.var_type == VarType::Binary)
                std::cout << " " << var.name << "\n";

        std::cout << "\n";
    }
} // namespace chen_solver
