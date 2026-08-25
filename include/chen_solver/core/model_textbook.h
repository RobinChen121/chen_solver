/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 13:03
 * Description: general model class for the models in the text books
 * 
 */

#ifndef CHEN_SOLVER_MODEL_TEXTBOOK_H
#define CHEN_SOLVER_MODEL_TEXTBOOK_H

#include <vector>

#include "chen_solver/core/matrix.h"

namespace chen_solver {
    struct SimplexTableau {
        int phase{};
        int iteration{};
        std::vector<int> basis;
        std::vector<std::vector<double> > coefficients;
        std::vector<double> rhs;
        std::vector<double> reduced_costs;
        double objective_value{};
        int entering_col{-1};
        int leaving_row{-1};
    };

    class ModelTextbook {
        // input parameters
        std::vector<double> obj_coe; // objective coefficients
        int obj_sense{}; // 0:min, 1: max
        int original_obj_sense{}; // keep the user's objective sense for reporting
        std::vector<std::vector<double> > con_lhs;
        std::vector<double> con_rhs;
        std::vector<int> constraint_sense; // 0:<=, 1: >=, 2: =
        std::vector<int> var_sign; // 0: >=, 1: <=, 2: unsigned
        std::vector<double> lb; // lower bound for each variable

        int enter_rule{}; // 0: Bland, 1: Dantzig, 2: lexicographic

        // middle parameters
        int m{}; // number of constraints
        int n0{}; // number of original variables
        int n{}; // number of variables after standardizing

        CSC A;
        std::vector<double> c; // objective coefficients after standardizing
        std::vector<double> b; // right hand sides after standardizing

        int num_slack{};
        int num_artificial{};
        // basis[i] = column index of the basic variable in row i
        std::vector<int> basis;
        // column type or var type:
        // 0 = structural / transformed structural
        // 1 = slack
        // 2 = artificial
        std::vector<int> column_type;
        // map_old_to_new[i] = starting column of original variable i
        // if var i is free, then:
        //   x_i = x_i^+ - x_i^-
        //   x_i^+ at map_old_to_new[i], x_i^- at map_old_to_new[i] + 1
        std::vector<int> map_old_to_new;
        // original variable sign for solution recovery
        std::vector<int> original_var_sign;
        std::vector<int> rhs_sign;

        // output
        int solution_status = {
            3
        }; // 0 optimal, 1 unbounded, 2 infeasible, 3 unsolved, 4 numerical/cycling
        double run_time = {};
        double objective_value = 0.0;
        std::vector<double> primal_solution_standard;
        std::vector<double> primal_solution_original;
        std::vector<double> constraint_dual_values;
        bool save_tableau_history = false;
        std::vector<SimplexTableau> tableau_history;

    public:
        ModelTextbook(const int obj_sense, const std::vector<double> &obj_coe,
                      const std::vector<std::vector<double> > &con_lhs,
                      const std::vector<double> &con_rhs,
                      const std::vector<int> &constraint_sense, const std::vector<int> &var_sign)
            : obj_coe(obj_coe), obj_sense(obj_sense), con_lhs(con_lhs), con_rhs(con_rhs),
              constraint_sense(constraint_sense), var_sign(var_sign) {
            m = static_cast<int>(con_lhs.size());
            n0 = static_cast<int>(var_sign.size());
            original_var_sign = var_sign;
            original_obj_sense = obj_sense;
        };
    };
}
#endif //CHEN_SOLVER_MODEL_TEXTBOOK_H
