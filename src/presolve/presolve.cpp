/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 13:03
 * Description: 
 * 
 */

#include "chen_solver/presolve/presolve.h"

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace chen_solver {
    namespace {
        struct RowState {
            std::string name;
            std::vector<LinearTerm> terms;
            double lb{-INF};
            double ub{INF};
            bool active{true};
        };

        struct ActivityRange {
            double min_activity{0.0};
            double max_activity{0.0};
            bool min_is_neg_inf{false};
            bool max_is_pos_inf{false};
        };

        [[nodiscard]] bool isNegInf(const double value) {
            return value <= -INF / 2.0;
        }

        [[nodiscard]] bool isPosInf(const double value) {
            return value >= INF / 2.0;
        }

        [[nodiscard]] bool nearlyEqual(const double lhs, const double rhs) {
            return std::abs(lhs - rhs) <= EPS;
        }

        [[nodiscard]] ActivityRange computeActivityRange(const RowState &row,
                                                         const std::vector<double> &var_lb,
                                                         const std::vector<double> &var_ub) {
            ActivityRange range;
            for (const auto &term: row.terms) {
                if (term.coef > 0.0) {
                    if (isNegInf(var_lb[term.col])) {
                        range.min_is_neg_inf = true;
                    } else if (!range.min_is_neg_inf) {
                        range.min_activity += term.coef * var_lb[term.col];
                    }

                    if (isPosInf(var_ub[term.col])) {
                        range.max_is_pos_inf = true;
                    } else if (!range.max_is_pos_inf) {
                        range.max_activity += term.coef * var_ub[term.col];
                    }
                } else {
                    if (isPosInf(var_ub[term.col])) {
                        range.min_is_neg_inf = true;
                    } else if (!range.min_is_neg_inf) {
                        range.min_activity += term.coef * var_ub[term.col];
                    }

                    if (isNegInf(var_lb[term.col])) {
                        range.max_is_pos_inf = true;
                    } else if (!range.max_is_pos_inf) {
                        range.max_activity += term.coef * var_lb[term.col];
                    }
                }
            }
            return range;
        }

        void fixVariable(const ChenInt col,
                         const double value,
                         std::vector<bool> &active_vars,
                         std::vector<double> &var_lb,
                         std::vector<double> &var_ub,
                         std::vector<double> &objective,
                         std::vector<bool> &variable_is_fixed,
                         std::vector<double> &fixed_values,
                         double &objective_offset,
                         bool &changed) {
            if (!active_vars[col]) {
                return;
            }

            active_vars[col] = false;
            variable_is_fixed[col] = true;
            fixed_values[col] = value;
            var_lb[col] = value;
            var_ub[col] = value;
            objective_offset += objective[col] * value;
            objective[col] = 0.0;
            changed = true;
        }

        [[nodiscard]] double chooseFeasibleValue(const double lb, const double ub) {
            if (!isNegInf(lb) && !isPosInf(ub)) {
                return lb;
            }
            if (!isNegInf(lb) && lb > 0.0) {
                return lb;
            }
            if (!isPosInf(ub) && ub < 0.0) {
                return ub;
            }
            return 0.0;
        }
    } // namespace

    PresolveReport presolveLinearProgram(const Model &model) {
        PresolveReport report;
        const auto &variables = model.variables();
        const auto &constraints = model.constraints();
        const auto num_vars = variables.size();
        const auto num_cons = constraints.size();

        std::vector<bool> active_vars(num_vars, true);
        std::vector<double> var_lb(num_vars, 0.0);
        std::vector<double> var_ub(num_vars, 0.0);
        std::vector<double> objective(num_vars, 0.0);
        std::vector<bool> variable_is_fixed(num_vars, false);
        std::vector<double> fixed_values(num_vars, 0.0);
        std::vector<RowState> rows;
        rows.reserve(num_cons);

        for (std::size_t col = 0; col < num_vars; ++col) {
            var_lb[col] = variables[col].lb;
            var_ub[col] = variables[col].ub;
            if (var_lb[col] > var_ub[col] + EPS) {
                report.result = PresolveResult::PrimalInfeasible;
                return report;
            }
        }

        for (const auto &[col, coef]: model.objectiveCoefficients()) {
            if (col < 0 || static_cast<std::size_t>(col) >= num_vars) {
                throw std::out_of_range("Invalid objective coefficient column index");
            }
            if (std::abs(coef) > EPS) {
                objective[col] = coef;
            }
        }

        for (const auto &constraint: constraints) {
            if (constraint.lb > constraint.ub + EPS) {
                report.result = PresolveResult::PrimalInfeasible;
                return report;
            }

            RowState row;
            row.name = constraint.name;
            row.lb = constraint.lb;
            row.ub = constraint.ub;
            row.terms.reserve(constraint.lhs.size());
            for (const auto &term: constraint.lhs) {
                if (term.col < 0 || static_cast<std::size_t>(term.col) >= num_vars) {
                    throw std::out_of_range("Invalid constraint column index");
                }
                if (std::abs(term.coef) > EPS) {
                    row.terms.push_back(term);
                }
            }
            rows.push_back(std::move(row));
        }

        double objective_offset = model.objectiveOffset();
        bool changed = false;
        bool pass_changed = false;

        do {
            pass_changed = false;

            for (ChenInt col = 0; col < static_cast<ChenInt>(num_vars); ++col) {
                if (!active_vars[col]) {
                    continue;
                }

                if (var_lb[col] > var_ub[col] + EPS) {
                    report.result = PresolveResult::PrimalInfeasible;
                    return report;
                }

                if (nearlyEqual(var_lb[col], var_ub[col])) {
                    fixVariable(col, var_lb[col], active_vars, var_lb, var_ub, objective,
                                variable_is_fixed, fixed_values, objective_offset, pass_changed);
                }
            }

            for (auto &row: rows) {
                if (!row.active) {
                    continue;
                }

                double fixed_shift = 0.0;
                std::vector<LinearTerm> filtered_terms;
                filtered_terms.reserve(row.terms.size());
                for (const auto &term: row.terms) {
                    if (!active_vars[term.col]) {
                        fixed_shift += term.coef * fixed_values[term.col];
                        pass_changed = true;
                        continue;
                    }
                    filtered_terms.push_back(term);
                }

                if (std::abs(fixed_shift) > EPS) {
                    if (!isNegInf(row.lb)) {
                        row.lb -= fixed_shift;
                    }
                    if (!isPosInf(row.ub)) {
                        row.ub -= fixed_shift;
                    }
                }
                row.terms = std::move(filtered_terms);

                if (row.lb > row.ub + EPS) {
                    report.result = PresolveResult::PrimalInfeasible;
                    return report;
                }

                if (row.terms.empty()) {
                    if ((!isNegInf(row.lb) && row.lb > EPS) || (
                            !isPosInf(row.ub) && row.ub < -EPS)) {
                        report.result = PresolveResult::PrimalInfeasible;
                        return report;
                    }
                    row.active = false;
                    pass_changed = true;
                    continue;
                }

                const auto activity = computeActivityRange(row, var_lb, var_ub);
                if (!activity.min_is_neg_inf && !isPosInf(row.ub) && activity.min_activity > row.ub
                    + EPS) {
                    report.result = PresolveResult::PrimalInfeasible;
                    return report;
                }
                if (!activity.max_is_pos_inf && !isNegInf(row.lb) && activity.max_activity < row.lb
                    - EPS) {
                    report.result = PresolveResult::PrimalInfeasible;
                    return report;
                }

                const bool lower_always_satisfied =
                        isNegInf(row.lb) || (
                            !activity.min_is_neg_inf && activity.min_activity >= row.lb - EPS);
                const bool upper_always_satisfied =
                        isPosInf(row.ub) || (
                            !activity.max_is_pos_inf && activity.max_activity <= row.ub + EPS);
                if (lower_always_satisfied && upper_always_satisfied) {
                    row.active = false;
                    pass_changed = true;
                    continue;
                }

                if (row.terms.size() != 1) {
                    continue;
                }

                const auto [col, coef] = row.terms.front();
                double new_lb = var_lb[col];
                double new_ub = var_ub[col];

                if (!isNegInf(row.lb)) {
                    if (coef > 0.0) {
                        new_lb = std::max(new_lb, row.lb / coef);
                    } else {
                        new_ub = std::min(new_ub, row.lb / coef);
                    }
                }
                if (!isPosInf(row.ub)) {
                    if (coef > 0.0) {
                        new_ub = std::min(new_ub, row.ub / coef);
                    } else {
                        new_lb = std::max(new_lb, row.ub / coef);
                    }
                }

                if (new_lb > new_ub + EPS) {
                    report.result = PresolveResult::PrimalInfeasible;
                    return report;
                }

                if (new_lb > var_lb[col] + EPS) {
                    var_lb[col] = new_lb;
                    pass_changed = true;
                }
                if (new_ub < var_ub[col] - EPS) {
                    var_ub[col] = new_ub;
                    pass_changed = true;
                }
            }

            std::vector<ChenInt> column_nnz(num_vars, 0);
            for (const auto &row: rows) {
                if (!row.active) {
                    continue;
                }
                for (const auto &term: row.terms) {
                    ++column_nnz[term.col];
                }
            }

            for (ChenInt col = 0; col < static_cast<ChenInt>(num_vars); ++col) {
                if (!active_vars[col] || column_nnz[col] != 0) {
                    continue;
                }

                const double coef = objective[col];
                if (std::abs(coef) <= EPS) {
                    const double value = chooseFeasibleValue(var_lb[col], var_ub[col]);
                    fixVariable(col, value, active_vars, var_lb, var_ub, objective,
                                variable_is_fixed, fixed_values, objective_offset, pass_changed);
                    continue;
                }

                double value = 0.0;
                switch (model.objectiveSense()) {
                    case ObjSense::Minimize:
                        if (coef > 0.0) {
                            if (isNegInf(var_lb[col])) {
                                report.result = PresolveResult::DualInfeasible;
                                return report;
                            }
                            value = var_lb[col];
                        } else {
                            if (isPosInf(var_ub[col])) {
                                report.result = PresolveResult::DualInfeasible;
                                return report;
                            }
                            value = var_ub[col];
                        }
                        break;
                    case ObjSense::Maximize:
                        if (coef > 0.0) {
                            if (isPosInf(var_ub[col])) {
                                report.result = PresolveResult::DualInfeasible;
                                return report;
                            }
                            value = var_ub[col];
                        } else {
                            if (isNegInf(var_lb[col])) {
                                report.result = PresolveResult::DualInfeasible;
                                return report;
                            }
                            value = var_lb[col];
                        }
                        break;
                }

                fixVariable(col, value, active_vars, var_lb, var_ub, objective,
                            variable_is_fixed, fixed_values, objective_offset, pass_changed);
            }

            changed = changed || pass_changed;
        } while (pass_changed);

        report.original_to_presolved_col.assign(num_vars, -1);
        report.variable_is_fixed = variable_is_fixed;
        report.fixed_values = fixed_values;
        report.objective_offset_shift = objective_offset - model.objectiveOffset();

        report.presolved_model.setObjectiveSense(model.objectiveSense());
        report.presolved_model.setObjectiveOffset(objective_offset);

        for (std::size_t old_col = 0; old_col < num_vars; ++old_col) {
            if (!active_vars[old_col]) {
                continue;
            }

            const auto new_col = report.presolved_model.addVariable(
                var_lb[old_col], var_ub[old_col], variables[old_col].var_type,
                variables[old_col].name);
            report.original_to_presolved_col[old_col] = new_col;
            if (std::abs(objective[old_col]) > EPS) {
                report.presolved_model.setObjectiveCoefficient(new_col, objective[old_col]);
            }
        }

        for (const auto &[name, terms, lb, ub, active]: rows) {
            if (!active) {
                continue;
            }

            std::vector<LinearTerm> new_terms;
            new_terms.reserve(terms.size());
            for (const auto &term: terms) {
                new_terms.push_back({report.original_to_presolved_col[term.col], term.coef});
            }
            report.presolved_model.addLinearConstraint(new_terms, lb, ub, name);
        }

        report.result = changed ? PresolveResult::Presolved : PresolveResult::NotPresolved;
        return report;
    }
} // namespace chen_solver
