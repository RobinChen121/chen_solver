#include <iostream>

#include "chen_solver/model.h"

int main() {
    chen_solver::Model model;
    model.set_objective_sense(chen_solver::ObjSense::Minimize);
    model.addVariable("x0", 0.0, 4.0);
    model.addVariable("x1", 0.0, 8.0);
    model.addLinearConstraint("capacity", {{0, 2.0}, {1, 1.0}}, -chen_solver::INF, 8.0);

    std::cout << "example model: vars=" << model.num_variables()
              << ", cons=" << model.numConstraints() << '\n';
    return 0;
}
