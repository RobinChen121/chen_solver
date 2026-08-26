/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/25, 17:01
 * Description: 
 *
 */

/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 P26/08/20, 13:03
 * Description: 
 *
 */

#include <iostream>

#include "../include/chen_solver/core/model.h"

int main() {
    chen_solver::ChenModel model;
    const auto x0 = model.addVar(0.0, 4.0, chen_solver::VarType::Continuous, "x0");
    const auto x1 = model.addVar(0.0, 8.0, chen_solver::VarType::Continuous, "x1");
    model.setObjective(3.0 * x0 + 2.0 * x1 + 1.0, chen_solver::ObjSense::Minimize);
    model.addLineConstr(2.0 * x0 + x1 <= 8.0, "capacity");
    model.addLineConstr(x0 - x1 == 0.0, "balance");

    std::cout << "example model: vars=" << model.numVariables()
              << ", cons=" << model.numConstraints() << '\n';
    return 0;
}
