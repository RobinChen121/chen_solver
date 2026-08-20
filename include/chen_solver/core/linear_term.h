#ifndef CHEN_SOLVER_LINEAR_TERM_H
#define CHEN_SOLVER_LINEAR_TERM_H

#include "chen_solver/config.h"

namespace chen_solver {

struct LinearTerm {
    ChenInt col{}; // column index
    double coef{};
};

} // namespace chen_solver

#endif //CHEN_SOLVER_LINEAR_TERM_H
