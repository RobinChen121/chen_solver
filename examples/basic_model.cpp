#include <iostream>

#include "../include/chen_solver/core/model.h"

int main() {
    chen_solver::Model model;
    const auto x0 = model.addVar("x0", 0.0, 4.0);
    const auto x1 = model.addVar("x1", 0.0, 8.0);
    model.setObjective(3.0 * x0 + 2.0 * x1 + 1.0, chen_solver::ObjSense::Minimize);
    model.addConstr("capacity", 2.0 * x0 + x1 <= 8.0);
    model.addConstr("balance", x0 - x1 == 0.0);

    std::cout << "example model: vars=" << model.numVariables()
              << ", cons=" << model.numConstraints() << '\n';
    return 0;
}
