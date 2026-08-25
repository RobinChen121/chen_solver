/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/25, 17:01
 * Description: Domain Specific Language (DSL) for modeling.
 *
 */

#include <cassert>

#include "chen_solver/core/model.h"

int main() {
    chen_solver::Model model;
    const auto x = model.addVar(0.0, 10.0, chen_solver::VarType::Continuous, "x");
    const auto y = model.addVar(1.0, 9.0, chen_solver::VarType::Continuous, "y");

    model.setObjective(2.0 * x + y + 5.0, chen_solver::ObjSense::Maximize);

    const auto c0 = model.addConstr(2.0 * x + 3.0 * y <= 10.0, "cap");
    const auto c1 = model.addConstr(x - y == 1.0);
    const auto c2 = model.addConstr(chen_solver::range(-1.0, x + y + 2.0, 4.0), "rng");
    const auto c3 = model.addConstr(3.0 <= x + y);

    assert(c0 == 0);
    assert(c1 == 1);
    assert(c2 == 2);
    assert(c3 == 3);
    assert(model.numVariables() == 2);
    assert(model.numConstraints() == 4);
    assert(model.objectiveSense() == chen_solver::ObjSense::Maximize);
    assert(model.getObjectiveCoefficient(x.col()) == 2.0);
    assert(model.getObjectiveCoefficient(y.col()) == 1.0);
    assert(model.objectiveOffset() == 5.0);

    const auto &constraints = model.constraints();
    assert(constraints[0].lb <= -chen_solver::INF / 2.0);
    assert(constraints[0].ub == 10.0);
    assert(constraints[0].lhs.size() == 2);
    assert(constraints[1].lb == 1.0);
    assert(constraints[1].ub == 1.0);
    assert(constraints[2].lb == -3.0);
    assert(constraints[2].ub == 2.0);
    assert(constraints[3].lb == 3.0);
    assert(constraints[3].ub >= chen_solver::INF / 2.0);
    assert(model.checkValid());
    return 0;
}
