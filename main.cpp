#include <iostream>

#include "include/chen_solver/model.h"
#include "include/chen_solver/config.h"

int main() {
    chen_solver::Model model;
    model.addVariable("x0", 0.0, 10.0);
    model.addLinearConstraint("c0", {{0, 1.0}}, 2.0, chen_solver::INF);

    std::cout << "chen_solver_cli: vars=" << model.num_variables()
              << ", cons=" << model.numConstraints() << '\n';
    return 0;
}
