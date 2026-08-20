#include <cassert>

#include "../include/chen_solver/core/model.h"

int main() {
    chen_solver::Model model;
    assert(model.modelStatus() == chen_solver::ModelStatus::Empty);

    const auto v0 = model.addVariable("x0", 0.0, 1.0, chen_solver::VarType::Binary);
    assert(v0 == 0);
    assert(model.numVariables() == 1);
    assert(model.modelStatus() == chen_solver::ModelStatus::Modified);

    const auto c0 = model.addLinearConstraint("cover", {{0, 1.0}}, 1.0, chen_solver::INF);
    assert(c0 == 0);
    assert(model.numConstraints() == 1);
    assert(model.checkValid());
    return 0;
}
