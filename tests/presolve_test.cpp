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
        chen_solver::ChenModel model;
        model.setObjectiveSense(chen_solver::ObjSense::Minimize);
        const auto x = model.addVar(1.0, 1.0, chen_solver::VarType::Continuous, "x");
        const auto y = model.addVar(0.0, 10.0, chen_solver::VarType::Continuous, "y");
        const auto z = model.addVar(0.0, 10.0, chen_solver::VarType::Continuous, "z");
        model.setObjectiveCoefficient(x.col(), 2.0);
        model.setObjectiveCoefficient(y.col(), 3.0);
        model.addLineConstr({{x.col(), 1.0}, {y.col(), 1.0}, {z.col(), 1.0}}, -chen_solver::INF, 5.0, "cap");

        const auto report = chen_solver::presolveLinearProgram(model);
        assert(report.result == chen_solver::PresolveResult::Presolved);
        assert(report.presolved_model.numVariables() == 2);
        assert(report.presolved_model.numConstraints() == 1);
        assert(report.objective_offset_shift == 2.0);
        assert(report.variable_is_fixed.size() == 3);
        assert(report.variable_is_fixed[0]);
        assert(report.fixed_values[0] == 1.0);
        assert(report.original_to_presolved_col[0] == -1);
        assert(report.original_to_presolved_col[1] == 0);
        assert(report.original_to_presolved_col[2] == 1);

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
        chen_solver::ChenModel model;
        const auto x = model.addVar(0.0, 1.0, chen_solver::VarType::Continuous, "x");
        model.addLineConstr({{x.col(), 1.0}}, 2.0, chen_solver::INF, "need_more");

        const auto report = chen_solver::presolveLinearProgram(model);
        assert(report.result == chen_solver::PresolveResult::PrimalInfeasible);
    }
    {
        chen_solver::ChenModel model;
        model.setObjectiveSense(chen_solver::ObjSense::Minimize);
        const auto x = model.addVar(0.0, chen_solver::INF, chen_solver::VarType::Continuous, "x");
        model.setObjectiveCoefficient(x.col(), -1.0);

        const auto report = chen_solver::presolveLinearProgram(model);
        assert(report.result == chen_solver::PresolveResult::DualInfeasible);
    }

    return 0;
}
