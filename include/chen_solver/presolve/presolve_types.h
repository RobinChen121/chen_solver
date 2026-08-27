/**
* Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/27, 09:03
 * Description: 没有将这个文件放到 presolve.h 中的原因是，
 * presolve_types.h 中定义的类型在 model.h 中也需要使用，
 * 而 model.h 又需要包含 presolve.h，
 * 这样就会导致循环依赖的问题。
 *
 */

#ifndef CHEN_SOLVER_PRESOLVE_TYPES_H
#define CHEN_SOLVER_PRESOLVE_TYPES_H

#include <string>

#include "chen_solver/config.h"

enum class PresolveResult : uint8_t {
    NotPresolved = 0,
    Presolved = 1,
    PrimalInfeasible = 2,
    DualInfeasible = 3,
};

enum class PresolveActionType : uint8_t {
    NormalizedObjectiveSense = 0, // Converted maximization objective to minimization form
    FixedVariable = 1, // Fixed a variable to a specific value due to constraints or bounds
    ShiftedConstraintBounds = 2, // Shifted the bounds of a constraint due to fixed variables
    RemovedEmptyConstraint = 3, // Removed a constraint that became empty after presolve
    RemovedRedundantConstraint = 4,
    // Removed a constraint that was redundant and did not affect the feasible region
    TightenedVariableLowerBound = 5,
    // Tightened the lower bound of a variable due to constraints or bounds
    TightenedVariableUpperBound = 6,
    // Tightened the upper bound of a variable due to constraints or bounds
};

struct PresolveAction {
    PresolveActionType type{PresolveActionType::FixedVariable};
    ChenInt variable_col{-1};
    std::string variable_name;
    std::string constraint_name;
    double value{0.0};
    double old_bound{0.0};
    double new_bound{0.0};
    double lower_shift{0.0};
    double upper_shift{0.0};
    double objective_shift{0.0};
    ObjSense old_objective_sense{ObjSense::Minimize};
    ObjSense new_objective_sense{ObjSense::Minimize};
};

#endif // CHEN_SOLVER_PRESOLVE_TYPES_H
