/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/25, 17:01
 * Description: 
 *
 */


#ifndef CHEN_SOLVER_LINEAR_CONSTRAINT_H
#define CHEN_SOLVER_LINEAR_CONSTRAINT_H

#include <string>
#include <vector>

#include "chen_solver/config.h"
#include "linear_term.h"

    struct LinearConstraint {
        ChenUInt id{};
        std::string name;
        std::vector<LinearTerm> lhs;
        double lb{-INF};
        double ub{INF};

        LinearConstraint() = default;

        // 预分配约束里的非零项数目，在模型规模较大时可以减少扩容开销。
        explicit LinearConstraint(const ChenInt reserve_terms) {
            lhs.reserve(static_cast<std::size_t>(reserve_terms));
        }
    };
#endif //CHEN_SOLVER_LINEAR_CONSTRAINT_H
