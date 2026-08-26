/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2020/08/20, 13:03
 * Description: A simple example same as Highs's introducing case.
 *
 */

#include <iostream>

#include "../include/chen_solver/core/model.h"

int main() {
    ChenEnv env;
    env.setOutputFlag(true);
    env.writeLog("basic_lp.log");
    ChenModel model(env);
    const auto x0 = model.addVar(0.0, 4.0, VarType::Continuous, "x0");
    const auto x1 = model.addVar(1.0, INF, VarType::Continuous, "x1");

    model.setObjective(x0 + x1 + 3.0, ObjSense::Minimize);
    model.addLineConstr(x1 <= 7.0, "c1");
    model.addLineConstr(5.0 <= x0 + 2.0 * x1, "c2");
    model.addLineConstr(x0 + 2.0 * x1 <= 15.0, "c3");
    model.addLineConstr(6.0 <= 3.0 * x0 + 2.0 * x1, "c4");

    // model.print();

    model.optimize();


    return 0;
}
