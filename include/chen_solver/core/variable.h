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
 * Created on: 2026/08/25, 17:01
 * Description: 
 *
 */

#ifndef CHEN_SOLVER_VARIABLE_H
#define CHEN_SOLVER_VARIABLE_H

#include <string>

#include "chen_solver/config.h"

    struct Variable {
        ChenUInt id{};
        std::string name;
        double lb{0.0};
        double ub{INF};
        VarType var_type{VarType::Continuous};
    };
#endif //CHEN_SOLVER_VARIABLE_H
