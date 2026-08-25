/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 13:03
 * Description: 
 *
 */

#ifndef CHEN_SOLVER_MODEL_H
#define CHEN_SOLVER_MODEL_H

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "chen_solver/api/model_builder_cpp.h"
#include "chen_solver/config.h"
#include "linear_constraint.h"
#include "variable.h"

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

        ChenUInt next_var_id = 0;
        ChenUInt next_con_id = 0;

    public:
        ChenInt addVariable(double lb = 0.0,
                            double ub = INF,
                            VarType var_type = VarType::Continuous,
                            const std::string &name = "");
        Var addVar(double lb = 0.0,
                   double ub = INF,
                   VarType var_type = VarType::Continuous,
                   const std::string &name = "");

        ChenInt addLinearConstraint(const std::vector<LinearTerm> &terms = {},
                                    double lb = -INF,
                                    double ub = INF,
                                    const std::string &name = "");
        ChenInt addConstr(const TempConstr &constraint, const std::string &name = "");

        [[nodiscard]] std::size_t numVariables() const noexcept;
        [[nodiscard]] std::size_t numConstraints() const noexcept;
        [[nodiscard]] ModelStatus modelStatus() const noexcept;

        [[nodiscard]] const std::vector<Variable> &variables() const noexcept { return variables_; }

        [[nodiscard]] const std::vector<LinearConstraint> &constraints() const noexcept {
            return constraints_;
        }

        [[nodiscard]] ObjSense objectiveSense() const noexcept { return objective_sense_; }

        [[nodiscard]] const std::map<ChenInt, double> &objectiveCoefficients() const noexcept {
            return objective_coef;
        }

        [[nodiscard]] double objectiveOffset() const noexcept { return objective_offset; }

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

        [[nodiscard]] double getObjectiveCoefficient(const ChenInt col) const {
            if (const auto it = objective_coef.find(col); it != objective_coef.end()) {
                return it->second;
            }
            return 0.0; // 如果没有设置系数，返回默认值 0.0
        }

        void setObjectiveCoefficient(const ChenInt col, const double coef) {
            if (std::abs(coef) <= EPS) {
                objective_coef.erase(col);
            } else {
                objective_coef[col] = coef;
            }
            status_ = ModelStatus::Modified;
        };

        void setObjectiveOffset(const double offset) {
            objective_offset = offset;
            status_ = ModelStatus::Modified;
        };

        void setObjective(const LinExpr &expression, ObjSense sense = ObjSense::Minimize);

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

        // 求解
        void solve();
    };
} // namespace chen_solver

#endif //CHEN_SOLVER_MODEL_H
