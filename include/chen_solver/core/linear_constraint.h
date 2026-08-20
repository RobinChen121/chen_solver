#ifndef CHEN_SOLVER_LINEAR_CONSTRAINT_H
#define CHEN_SOLVER_LINEAR_CONSTRAINT_H

#include <string>
#include <vector>

#include "chen_solver/config.h"
#include "linear_term.h"

namespace chen_solver {

struct LinearConstraint {
    ConId id{};
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

} // namespace chen_solver

#endif //CHEN_SOLVER_LINEAR_CONSTRAINT_H
