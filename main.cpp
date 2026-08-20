#include <iostream>

#include "include/chen_solver/core/model.h"

int main() {
    chen_solver::Model model;
    const auto x0 = model.addVar("x0", 0.0, 10.0);
    const auto x1 = model.addVar("x1", 0.0, 10.0);
    model.setObjective(2.0 * x0 + x1, chen_solver::ObjSense::Minimize);
    model.addConstr("c0", x0 + x1 >= 2.0);

    std::cout << "chen_solver_cli: vars=" << model.numVariables()
              << ", cons=" << model.numConstraints() << '\n';
    return 0;
}
