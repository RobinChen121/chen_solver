#ifndef CHEN_SOLVER_MODEL_H
#define CHEN_SOLVER_MODEL_H

#include <string>
#include <vector>
#include <map>

#include "chen_solver/config.h"
#include "chen_solver/linear_constraint.h"
#include "chen_solver/variable.h"

namespace chen_solver {
    // general model for the industrial solver
    class Model {
        std::vector<Variable> variables_;
        std::vector<LinearConstraint> constraints_;

        std::string model_name_;
        ObjSense objective_sense_{ObjSense::Minimize};
        // sparse objective coefficients for each column index
        std::map<ChenInt, double> objective_coef;
        double objective_offset = 0.0;
        ModelStatus status_{ModelStatus::Empty};

        // map from var name to its variable index
        std::map<std::string, ChenInt> name_to_varIndex;
        // map from constraint name to its constraint index
        std::map<std::string, ChenInt> name_to_conIndex;

        VarId next_var_id = 0;
        ConId next_con_id = 0;

    public:
        void addVariable(const std::string &name = "",
                         double lb = 0.0,
                         double ub = INF,
                         VarType var_type = VarType::Continuous);

        void addLinearConstraint(const std::string &name = "",
                                 const std::vector<LinearTerm> &terms = {},
                                 double lb = -INF,
                                 double ub = INF);

        [[nodiscard]] std::size_t numVariables() const noexcept;
        [[nodiscard]] std::size_t numConstraints() const noexcept;
        [[nodiscard]] ModelStatus modelStatus() const noexcept;

        void setVariableLb(const ChenInt col, const double lb) {
            variables_[col].lb = lb;
            status_ = ModelStatus::Modified;
        };

        void setVariableUb(const ChenInt col, const double ub) {
            variables_[col].ub = ub;
            status_ = ModelStatus::Modified;
        };

        void setVariableType(const ChenInt col, const VarType var_type) {
            variables_[col].var_type = var_type;
            status_ = ModelStatus::Modified;
        };

        void setObjectiveSense(const ObjSense sense) {
            objective_sense_ = sense;
            status_ = ModelStatus::Modified;
        };

        double getObjectiveCoefficient(const ChenInt col) const {
            if (const auto it = objective_coef.find(col); it != objective_coef.end()) {
                return it->second;
            }
            return 0.0; // 如果没有设置系数，返回默认值 0.0
        }

        void setObjectiveCoefficient(const ChenInt col, const double coef) {
            objective_coef[col] = coef;
            status_ = ModelStatus::Modified;
        };

        void setModelName(const std::string &name) {
            model_name_ = name;
        }

        // 检查模型是否有效
        [[nodiscard]] bool checkValid() const;
        [[nodiscard]] ChenInt nameToVarIndex(const std::string &name) const;
        ChenInt findOrCreateVariable(const std::string &name);

        // 打印模型信息
        void print() const;
        void printLinearExpression(const std::vector<LinearTerm> &terms) const;
        void printObjective() const;
        void printConstraints() const;
        void printBounds() const;
        void printIntegers() const;
        void printBinaries() const;
    };

    // general model class for the models in the text books
    class ModelTextbook {
        // input parameters
        std::vector<double> obj_coe; // objective coefficients
        int obj_sense; // 0:min, 1: max
        int original_obj_sense; // keep the user's objective sense for reporting
        std::vector<std::vector<double> > con_lhs;
        std::vector<double> con_rhs;
        std::vector<int> constraint_sense; // 0:<=, 1: >=, 2: =
        std::vector<int> var_sign; // 0: >=, 1: <=, 2: unsigned
        std::vector<double> lb; // lower bound for each variable

        int enter_rule{}; // 0: Bland, 1: Dantzig, 2: lexicographic

        // middle parameters
        int m; // number of constraints
        int n0; // number of original variables
        int n{}; // number of variables after standardizing
    };
} // namespace chen_solver

#endif //CHEN_SOLVER_MODEL_H
