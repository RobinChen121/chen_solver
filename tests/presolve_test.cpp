/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/25, 17:01
 * Description: 
 *
 */

#include <cassert>

#include "chen_solver/presolve/presolve.h"

int main()
{
    {
        ChenModel model;
        model.setObjectiveSense(ObjSense::Minimize);
        const auto x = model.addVar(1.0, 1.0, VarType::Continuous, "x");
        const auto y = model.addVar(0.0, 10.0, VarType::Continuous, "y");
        const auto z = model.addVar(0.0, 10.0, VarType::Continuous, "z");
        model.setObjectiveCoefficient(x.col(), 2.0);
        model.setObjectiveCoefficient(y.col(), 3.0);
        model.addLineConstr({{x.col(), 1.0}, {y.col(), 1.0}, {z.col(), 1.0}}, -INF, 5.0, "cap");

        const auto report = presolveLinearProgram(model);
        assert(report.result == PresolveResult::Presolved);
        assert(report.presolved_model.numVariables() == 2);
        assert(report.presolved_model.numConstraints() == 1);
        assert(report.objective_offset_shift == 2.0);
        assert(report.variable_is_fixed.size() == 3);
        assert(report.variable_is_fixed[0]);
        assert(report.fixed_values[0] == 1.0);
        assert(report.original_to_presolved_col[0] == -1);
        assert(report.original_to_presolved_col[1] == 0);
        assert(report.original_to_presolved_col[2] == 1);
        assert(report.actions.size() == 2);
        assert(report.actions[0].type == PresolveActionType::FixedVariable);
        assert(report.actions[0].variable_name == "x");
        assert(report.actions[0].value == 1.0);
        assert(report.actions[1].type == PresolveActionType::ShiftedConstraintBounds);
        assert(report.actions[1].constraint_name == "cap");

        const auto& vars = report.presolved_model.variables();
        const auto& cons = report.presolved_model.constraints();
        assert(vars[0].name == "y");
        assert(vars[1].name == "z");
        assert(report.presolved_model.getObjectiveCoefficient(0) == 3.0);
        assert(report.presolved_model.getObjectiveCoefficient(1) == 0.0);
        assert(report.presolved_model.objectiveOffset() == 2.0);
        assert(cons[0].lhs.size() == 2);
        assert(cons[0].lhs[0].col == 0);
        assert(cons[0].lhs[1].col == 1);
        assert(cons[0].ub == 4.0);
    }
    {
        ChenModel model;
        const auto x = model.addVar(0.0, 1.0, VarType::Continuous, "x");
        model.addLineConstr({{x.col(), 1.0}}, 2.0, INF, "need_more");

        const auto report = presolveLinearProgram(model);
        assert(report.result == PresolveResult::PrimalInfeasible);
    }
    {
        ChenModel model;
        model.setObjectiveSense(ObjSense::Minimize);
        const auto x = model.addVar(0.0, INF, VarType::Continuous, "x");
        model.setObjectiveCoefficient(x.col(), -1.0);

        const auto report = presolveLinearProgram(model);
        assert(report.result == PresolveResult::DualInfeasible);
    }
    {
        ChenModel model;
        model.setObjective(2.0 + 3.0 * model.addVar(0.0, 4.0, VarType::Continuous, "x"),
                           ObjSense::Maximize);

        const auto report = presolveLinearProgram(model);
        assert(report.result == PresolveResult::Presolved);
        assert(report.presolved_model.objectiveSense() == ObjSense::Minimize);
        assert(report.actions.size() >= 2);
        assert(report.actions[0].type == PresolveActionType::NormalizedObjectiveSense);
        assert(report.actions[0].old_objective_sense == ObjSense::Maximize);
        assert(report.actions[0].new_objective_sense == ObjSense::Minimize);
    }

    return 0;
}
