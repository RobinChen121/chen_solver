/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 13:03
 * Description: 
 *
 */

#include "../../include/chen_solver/core/model.h"

#include "chen_solver/presolve/presolve.h"
#include "chen_solver/util/logger.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace
{
    [[nodiscard]] std::string formatRangeValue(const double value)
    {
        std::ostringstream oss;
        // 使用科学计数法表示浮点数，并设置精度为 0, 会导致 1.5+e01 (15) 显示为 2+e01 (20)
        oss << std::scientific << std::setprecision(0) << value;
        return oss.str();
    }

    [[nodiscard]] std::pair<double, double> finiteRange(const std::vector<double>& values)
    {
        double min_value = std::numeric_limits<double>::infinity();
        double max_value = -std::numeric_limits<double>::infinity();
        for (const double value : values)
        {
            if (!std::isfinite(value) || std::abs(value) >= INF / 2.0)
            {
                continue;
            }
            min_value = std::min(min_value, value);
            max_value = std::max(max_value, value);
        }
        if (!std::isfinite(min_value) || !std::isfinite(max_value))
        {
            return {0.0, 0.0};
        }
        return {min_value, max_value};
    }

    [[nodiscard]] std::string formatPresolveValue(const double value)
    {
        if (value <= -INF / 2.0)
        {
            return "-inf";
        }
        if (value >= INF / 2.0)
        {
            return "inf";
        }
        std::ostringstream oss;
        oss << std::scientific << std::setprecision(6) << value;
        return oss.str();
    }

    [[nodiscard]] std::string describePresolveAction(const PresolveAction& action)
    {
        switch (action.type)
        {
        case PresolveActionType::FixedVariable:
            return "Presolve fixed variable " + action.variable_name + " = " +
                formatPresolveValue(action.value) + " (" + action.detail + ")";
        case PresolveActionType::ShiftedConstraintBounds:
            return "Presolve shifted constraint " + action.constraint_name + " by [" +
                formatPresolveValue(action.lower_shift) + ", " +
                formatPresolveValue(action.upper_shift) + "] (" + action.detail + ")";
        case PresolveActionType::RemovedEmptyConstraint:
        case PresolveActionType::RemovedRedundantConstraint:
            return "Presolve removed constraint " + action.constraint_name + " (" +
                action.detail + ")";
        case PresolveActionType::TightenedVariableLowerBound:
            return "Presolve tightened lower bound of " + action.variable_name + " from " +
                formatPresolveValue(action.old_bound) + " to " +
                formatPresolveValue(action.new_bound) + " (" + action.detail + ")";
        case PresolveActionType::TightenedVariableUpperBound:
            return "Presolve tightened upper bound of " + action.variable_name + " from " +
                formatPresolveValue(action.old_bound) + " to " +
                formatPresolveValue(action.new_bound) + " (" + action.detail + ")";
        case PresolveActionType::NormalizedObjectiveSense:
            return "Presolve converted objective sense from " +
                std::string(action.old_objective_sense == ObjSense::Minimize ? "Minimize" : "Maximize") +
                " to " +
                std::string(action.new_objective_sense == ObjSense::Minimize ? "Minimize" : "Maximize") +
                " (" + action.detail + ")";
        }
        return "Presolve applied an unrecognized action";
    }

    void logModelStatistics(const ChenModel& model)
    {
        const std::size_t rows = model.numConstraints();
        const std::size_t cols = model.numVariables();

        std::size_t nonzero_count = 0;
        double matrix_min = std::numeric_limits<double>::infinity();
        double matrix_max = -std::numeric_limits<double>::infinity();

        for (const auto& con : model.constraints())
        {
            for (const auto& [col, coef] : con.lhs)
            {
                if (std::abs(coef) <= EPS)
                {
                    continue;
                }
                ++nonzero_count;
                const double abs_coef = std::abs(coef);
                matrix_min = std::min(matrix_min, abs_coef);
                matrix_max = std::max(matrix_max, abs_coef);
            }
        }

        std::vector<double> cost_values;
        cost_values.reserve(model.objectiveCoefficients().size());
        for (const auto& coef : model.objectiveCoefficients() | std::views::values)
        {
            if (std::abs(coef) > EPS)
            {
                cost_values.push_back(std::abs(coef));
            }
        }

        std::vector<double> bound_values;
        bound_values.reserve(cols * 2);
        for (const auto& var : model.variables())
        {
            if (std::isfinite(var.lb))
            {
                bound_values.push_back(var.lb);
            }
            if (std::isfinite(var.ub))
            {
                bound_values.push_back(var.ub);
            }
        }

        std::vector<double> rhs_values;
        rhs_values.reserve(rows * 2);
        for (const auto& con : model.constraints())
        {
            if (std::isfinite(con.lb))
            {
                rhs_values.push_back(con.lb);
            }
            if (std::isfinite(con.ub))
            {
                rhs_values.push_back(con.ub);
            }
        }

        // ReSharper disable once CppUseStructuredBinding
        const auto cost_range = finiteRange(cost_values);
        // ReSharper disable once CppUseStructuredBinding
        const auto bound_range = finiteRange(bound_values);
        // ReSharper disable once CppUseStructuredBinding
        const auto rhs_range = finiteRange(rhs_values);

        auto& logger = Logger::instance();
        logger.info(
            "Optimize a model with " + std::to_string(cols) +
            " cols (variables); " + std::to_string(rows) + " rows (constraints); " +
            std::to_string(nonzero_count) + " nonzeros");
        logger.info("Coefficient ranges:");
        logger.info(
            "  Matrix (constraint left-hand side coefficients)  [" + formatRangeValue(
                matrix_min == std::numeric_limits<double>::infinity()
                    ? 0.0
                    : matrix_min) +
            ", " + formatRangeValue(matrix_max == -std::numeric_limits<double>::infinity()
                                        ? 0.0
                                        : matrix_max) + "]");
        logger.info(
            "  Cost (objective coefficients)    [" + formatRangeValue(cost_range.first) + ", " +
            formatRangeValue(
                cost_range.second) + "]");
        logger.info(
            "  Bound (variable bounds)   [" + formatRangeValue(bound_range.first) + ", " +
            formatRangeValue(
                bound_range.second) + "]");
        logger.info(
            "  RHS (constraint right-hand side coefficients)     [" +
            formatRangeValue(rhs_range.first) + ", " + formatRangeValue(
                rhs_range.second) + "]\n");
    }
} // namespace

void ChenModel::clearPresolveData() noexcept
{
    presolve_result_ = PresolveResult::NotPresolved;
    presolve_objective_offset_shift_ = 0.0;
    original_to_presolved_col_.clear();
    variable_is_fixed_.clear();
    fixed_values_.clear();
    presolve_actions_.clear();
}

void ChenModel::storePresolveReport(const PresolveReport& report)
{
    presolve_result_ = report.result;
    presolve_objective_offset_shift_ = report.objective_offset_shift;
    original_to_presolved_col_ = report.original_to_presolved_col;
    variable_is_fixed_ = report.variable_is_fixed;
    fixed_values_ = report.fixed_values;
    presolve_actions_ = report.actions;
}

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
    clearPresolveData();
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
    clearPresolveData();
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

    clearPresolveData();
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
    std::cout << std::string(50, '*') << std::endl;
    printObjective();
    printConstraints();
    printBounds();
    printIntegers();
    printBinaries();
    std::cout << "End\n";
    std::cout << std::string(50, '*') << std::endl;
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
    if (!checkValid())
    {
        throw std::runtime_error(
            "Model is invalid. Please check variable bounds and constraint bounds.");
        return;
    }

    // log header
    if (env_.outputFlag())
    {
        // 接受一个返回引用的函数时，使用 & 引用变量接受，避免临时拷贝
        auto& logger = Logger::instance();
        if (env_.hasLogFile())
        {
            logger.writeLogFile(env_.logFilePath());
        }

        const std::string separator(50, '*');
        logger.info(separator);

        // The solver banner lives in logger.cpp so model.cpp stays focused on model logic.
        logger.logHeader(Logger::getInfo());
        logger.info(separator);
        logger.info("Objective sense: " +
            std::string(objective_sense_ == ObjSense::Minimize ? "Minimize" : "Maximize"));
        // logger.info("solve started");
        logModelStatistics(*this);
        logger.info(separator);
    }

    // presolve
    clearPresolveData();
    // *this is a reference to the current ChenModel instance, which is passed to presolveLP
    const auto presolve_report = presolveLP(*this);
    storePresolveReport(presolve_report);

    if (env_.outputFlag())
    {
        auto& logger = Logger::instance();
        const std::string separator(50, '*');
        logger.info(separator);
        logger.info("Presolve started");
        for (const auto& action : presolve_actions_)
        {
            logger.info(describePresolveAction(action));
        }

        switch (presolve_result_)
        {
        case PresolveResult::NotPresolved:
            logger.info("Presolve kept the original model.");
            break;
        case PresolveResult::Presolved:
            logger.info(
                "Presolve reduced the model to " +
                std::to_string(presolve_report.presolved_model.numVariables()) + " cols and " +
                std::to_string(presolve_report.presolved_model.numConstraints()) + " rows.");
            break;
        case PresolveResult::PrimalInfeasible:
            logger.warn("Presolve detected primal infeasibility.");
            break;
        case PresolveResult::DualInfeasible:
            logger.warn("Presolve detected dual infeasibility.");
            break;
        }
    }
}
