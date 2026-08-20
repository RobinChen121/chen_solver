#include <iostream>

#include "../include/chen_solver/core/model.h"

int main() {
    chen_solver::Model model;
    model.setObjectiveSense(chen_solver::ObjSense::Minimize);
    const auto x0 = model.addVariable("x0", 0.0, 4.0);
    const auto x1 = model.addVariable("x1", 0.0, 8.0);
    model.addLinearConstraint("capacity", {{x0, 2.0}, {x1, 1.0}}, -chen_solver::INF, 8.0);

    std::cout << "example model: vars=" << model.numVariables()
              << ", cons=" << model.numConstraints() << '\n';
    return 0;
}
