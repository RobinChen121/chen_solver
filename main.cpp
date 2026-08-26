/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/25, 17:01
 * Description: 
 *
 */

#include <iostream>

#include "include/chen_solver/core/model.h"

int main() {
    ChenModel model;

    const auto x0 = model.addVar(0.0, 4.0, VarType::Continuous, "x0");
    const auto x1 = model.addVar(1.0, INF, VarType::Continuous, "x1");

    model.setObjective(x0 + x1 + 3.0, ObjSense::Minimize);

    model.addLineConstr(x1 <= 7.0, "x1_upper");
    model.addLineConstr(5.0 <= x0 + 2.0 * x1, "x0_x1_lower");
    model.addLineConstr(x0 + 2.0 * x1 <= 15.0, "x0_x1_upper");
    model.addLineConstr(6.0 <= 3.0 * x0 + 2.0 * x1, "x0_x1_rhs");

    std::cout << "chen_solver_cli: vars=" << model.numVariables()
              << ", cons=" << model.numConstraints() << '\n';

    model.optimize();
    return 0;
}
