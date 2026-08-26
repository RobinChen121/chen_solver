/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/25, 17:01
 * Description: 
 * 
 */

#ifndef CHEN_SOLVER_PRESOLVE_H
#define CHEN_SOLVER_PRESOLVE_H

#include <vector>

#include "chen_solver/core/model.h"

    enum class PresolveResult : uint8_t
    {
        NotPresolved = 0,
        Presolved = 1,
        PrimalInfeasible = 2,
        DualInfeasible = 3,
    };

    struct PresolveReport
    {
        PresolveResult result{PresolveResult::NotPresolved};
        ChenModel presolved_model;
        double objective_offset_shift{0.0};
        std::vector<ChenInt> original_to_presolved_col;
        std::vector<bool> variable_is_fixed;
        std::vector<double> fixed_values;

        [[nodiscard]] bool hasReducedModel() const noexcept
        {
            return result == PresolveResult::NotPresolved || result == PresolveResult::Presolved;
        }
    };

    [[nodiscard]] PresolveReport presolveLinearProgram(const ChenModel& model);

#endif //CHEN_SOLVER_PRESOLVE_H
