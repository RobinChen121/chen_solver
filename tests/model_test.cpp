/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/25, 17:01
 * Description: 
 *
 */

#include <cassert>

#include "../include/chen_solver/core/model.h"

int main()
{
    ChenModel model;
    assert(model.modelStatus() == ModelStatus::Empty);

    const auto v0 = model.addVar(0.0, 1.0, VarType::Binary, "x0");
    assert(v0.col() == 0);
    assert(model.numVariables() == 1);
    assert(model.modelStatus() == ModelStatus::Modified);

    const auto v1 = model.addVar(0, INF, VarType::Continuous, "x2");

    const auto c0 = model.addLineConstr({{v0.col(), 1.0}}, 1.0, INF, "cover");
    assert(c0 == 0);
    const auto c1 = model.addLineConstr(v0 + v1 <= 10.0, "sum");
    assert(model.numConstraints() == 2);
    assert(model.checkValid());
    return 0;
}
