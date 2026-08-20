/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/8/20, 10:59
 * Description: 
 * 
 */

#ifndef CHEN_SOLVER_PRESOLVE_H
#define CHEN_SOLVER_PRESOLVE_H

#include <vector>

#include "chen_solver/core/model.h"

namespace chen_solver
{
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
        Model presolved_model;
        double objective_offset_shift{0.0};
        std::vector<ChenInt> original_to_presolved_col;
        std::vector<bool> variable_is_fixed;
        std::vector<double> fixed_values;

        [[nodiscard]] bool hasReducedModel() const noexcept
        {
            return result == PresolveResult::NotPresolved || result == PresolveResult::Presolved;
        }
    };

    [[nodiscard]] PresolveReport presolveLinearProgram(const Model& model);
}

#endif //CHEN_SOLVER_PRESOLVE_H
