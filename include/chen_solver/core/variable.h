#ifndef CHEN_SOLVER_VARIABLE_H
#define CHEN_SOLVER_VARIABLE_H

#include <string>

#include "chen_solver/config.h"

namespace chen_solver {

struct Variable {
    VarId id{};
    std::string name;
    double lb{0.0};
    double ub{INF};
    VarType var_type{VarType::Continuous};
};

} // namespace chen_solver

#endif //CHEN_SOLVER_VARIABLE_H
