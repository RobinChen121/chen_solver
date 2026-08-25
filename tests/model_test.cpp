/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/25, 17:01
 * Description: 
 *
 */

#include <cassert>

#include "../include/chen_solver/core/model.h"

int main() {
    chen_solver::Model model;
    assert(model.modelStatus() == chen_solver::ModelStatus::Empty);

    const auto v0 = model.addVariable(0.0, 1.0, chen_solver::VarType::Binary, "x0");
    assert(v0 == 0);
    assert(model.numVariables() == 1);
    assert(model.modelStatus() == chen_solver::ModelStatus::Modified);

    const auto c0 = model.addLinearConstraint({{0, 1.0}}, 1.0, chen_solver::INF, "cover");
    assert(c0 == 0);
    assert(model.numConstraints() == 1);
    assert(model.checkValid());
    return 0;
}
