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
    assert(c1 == 1);
    assert(model.numConstraints() == 2);
    assert(model.checkValid());
    model.optimize();
    assert(model.presolveResult() == PresolveResult::Presolved);
    assert(model.variableIsFixed().size() == 2);
    assert(model.variableIsFixed()[0]);
    assert(model.variableIsFixed()[1]);
    assert(model.fixedValues()[0] == 1.0);
    assert(model.originalToPresolvedCol()[0] == -1);
    assert(model.originalToPresolvedCol()[1] == -1);
    bool has_fixed_action = false;
    bool has_shift_action = false;
    for (const auto& action : model.presolveActions())
    {
        if (action.type == PresolveActionType::FixedVariable)
        {
            has_fixed_action = true;
        }
        if (action.type == PresolveActionType::ShiftedConstraintBounds)
        {
            has_shift_action = true;
        }
    }
    assert(has_fixed_action);
    assert(has_shift_action);
    model.setObjectiveOffset(2.0);
    assert(model.presolveActions().empty());
    assert(model.variableIsFixed().empty());
    return 0;
}
