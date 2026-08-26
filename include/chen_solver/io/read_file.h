/**
 * Created by Zhen Chen on 2026/8/19.
 * Email: chen.zhen5526@gmail.com
 * Description:
 *
 *
 */

#ifndef CHEN_SOLVER_READ_FILE_H
#define CHEN_SOLVER_READ_FILE_H

#include <string>

#include "chen_solver/core/model.h"

    ChenModel readLP(const std::string &path);

    ChenModel readMPS(const std::string &path);

    ChenModel read(const std::string &path);

#endif //CHEN_SOLVER_READ_FILE_H
