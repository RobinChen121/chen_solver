#include <iostream>

#include "include/chen_solver/core/model.h"

int main() {
    chen_solver::Model model;
    const auto x0 = model.addVar(0.0, 10.0, chen_solver::VarType::Continuous, "x0");
    const auto x1 = model.addVar(0.0, 10.0, chen_solver::VarType::Continuous, "x1");
    model.setObjective(2.0 * x0 + x1, chen_solver::ObjSense::Minimize);
    model.addConstr(x0 + x1 >= 2.0, "c0");

    std::cout << "chen_solver_cli: vars=" << model.numVariables()
              << ", cons=" << model.numConstraints() << '\n';
    return 0;
}
