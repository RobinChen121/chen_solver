#include <iostream>

#include "include/chen_solver/core/model.h"

int main() {
    chen_solver::Model model;
    const auto x0 = model.addVariable("x0", 0.0, 10.0);
    model.addLinearConstraint("c0", {{x0, 1.0}}, 2.0, chen_solver::INF);

    std::cout << "chen_solver_cli: vars=" << model.numVariables()
              << ", cons=" << model.numConstraints() << '\n';
    return 0;
}
