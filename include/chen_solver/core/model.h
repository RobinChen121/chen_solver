/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 13:03
 * Description: 
 *
 */

#ifndef CHEN_SOLVER_MODEL_H
#define CHEN_SOLVER_MODEL_H

#include <map>
#include <string>
#include <vector>

#include "chen_solver/api/model_builder_cpp.h"
#include "chen_solver/config.h"
#include "chen_solver/presolve/presolve_types.h"
#include "env.h"
#include "linear_constraint.h"
#include "variable.h"

struct PresolveReport;

// general model for the industrial solver
class ChenModel
{
    ChenEnv env_; // 等价于 ChenEnv env_{}，对于类类型，两者都会调用默认构造函数 ChenEnv()

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
    PresolveResult presolve_result_{PresolveResult::NotPresolved};
    double presolve_objective_offset_shift_{0.0};
    std::vector<ChenInt> original_to_presolved_col_;
    std::vector<bool> variable_is_fixed_;
    std::vector<double> fixed_values_;
    std::vector<PresolveAction> presolve_actions_;

    // 内部实现用 addVariable
    ChenInt addVariable(double lb = 0.0,
                        double ub = INF,
                        VarType var_type = VarType::Continuous,
                        const std::string& name = "");
    // 内部实现用 addLinearConstraint
    ChenInt addLinearConstraint(const std::vector<LinearTerm>& terms = {},
                                double lb = -INF,
                                double ub = INF,
                                const std::string& name = "");
    void clearPresolveData() noexcept;
    void storePresolveReport(const PresolveReport& report);

public:
    // Default constructor uses default ChenEnv parameters.
    ChenModel() = default;
    // Pass a ChenEnv to configure solver parameters before solving.
    explicit ChenModel(const ChenEnv& env) : env_(env)
    {
    }

    [[nodiscard]] const ChenEnv& env() const noexcept { return env_; }

    // 外部接口用 addVar，返回 Var 对象
    Var addVar(double lb = 0.0,
               double ub = INF,
               VarType var_type = VarType::Continuous,
               const std::string& name = "");

    // 外部接口 addLineConstr 有两个实现，支持直接传入 LinearTerm 向量，或者传入 TempConstr 对象
    ChenInt addLineConstr(const std::vector<LinearTerm>& terms = {},
                          double lb = -INF,
                          double ub = INF,
                          const std::string& name = "");
    // 外部接口 addLineConstr 有两个实现，支持直接传入 LinearTerm 向量，或者传入 TempConstr 对象
    ChenInt addLineConstr(const TempConstr& constraint, const std::string& name = "");

    [[nodiscard]] std::size_t numVariables() const noexcept;
    [[nodiscard]] std::size_t numConstraints() const noexcept;
    [[nodiscard]] ModelStatus modelStatus() const noexcept;

    [[nodiscard]] const std::vector<Variable>& variables() const noexcept { return variables_; }

    [[nodiscard]] const std::vector<LinearConstraint>& constraints() const noexcept
    {
        return constraints_;
    }

    [[nodiscard]] ObjSense objectiveSense() const noexcept { return objective_sense_; }

    [[nodiscard]] const std::map<ChenInt, double>& objectiveCoefficients() const noexcept
    {
        return objective_coef;
    }

    [[nodiscard]] double objectiveOffset() const noexcept { return objective_offset; }
    [[nodiscard]] PresolveResult presolveResult() const noexcept { return presolve_result_; }

    [[nodiscard]] double presolveObjectiveOffsetShift() const noexcept
    {
        return presolve_objective_offset_shift_;
    }

    [[nodiscard]] const std::vector<ChenInt>& originalToPresolvedCol() const noexcept
    {
        return original_to_presolved_col_;
    }

    [[nodiscard]] const std::vector<bool>& variableIsFixed() const noexcept
    {
        return variable_is_fixed_;
    }

    [[nodiscard]] const std::vector<double>& fixedValues() const noexcept
    {
        return fixed_values_;
    }

    [[nodiscard]] const std::vector<PresolveAction>& presolveActions() const noexcept
    {
        return presolve_actions_;
    }

    void setVariableLb(const ChenInt col, const double lb)
    {
        variables_[col].lb = lb;
        clearPresolveData();
        status_ = ModelStatus::Modified;
    };

    void setVariableUb(const ChenInt col, const double ub)
    {
        variables_[col].ub = ub;
        clearPresolveData();
        status_ = ModelStatus::Modified;
    };

    void setVariableType(const ChenInt col, const VarType var_type)
    {
        variables_[col].var_type = var_type;
        clearPresolveData();
        status_ = ModelStatus::Modified;
    };

    void setObjectiveSense(const ObjSense sense)
    {
        objective_sense_ = sense;
        clearPresolveData();
        status_ = ModelStatus::Modified;
    };

    [[nodiscard]] double getObjectiveCoefficient(const ChenInt col) const
    {
        if (const auto it = objective_coef.find(col); it != objective_coef.end())
        {
            return it->second;
        }
        return 0.0; // 如果没有设置系数，返回默认值 0.0
    }

    void setObjectiveCoefficient(const ChenInt col, const double coef)
    {
        if (std::abs(coef) <= EPS)
        {
            objective_coef.erase(col);
        }
        else
        {
            objective_coef[col] = coef;
        }
        clearPresolveData();
        status_ = ModelStatus::Modified;
    };

    void setObjectiveOffset(const double offset)
    {
        objective_offset = offset;
        clearPresolveData();
        status_ = ModelStatus::Modified;
    };

    void setObjective(const LinExpr& expression, ObjSense sense = ObjSense::Minimize);

    void setModelName(const std::string& name)
    {
        model_name_ = name;
    }

    // 检查模型是否有效
    [[nodiscard]] bool checkValid() const;
    [[nodiscard]] ChenInt nameToVarIndex(const std::string& name) const;
    ChenInt findOrCreateVariable(const std::string& name);

    // 打印模型信息
    void print() const;
    void printLinearExpression(const std::vector<LinearTerm>& terms) const;
    void printObjective() const;
    void printConstraints() const;
    void printBounds() const;
    void printIntegers() const;
    void printBinaries() const;

    // 求解
    void optimize();
};
#endif //CHEN_SOLVER_MODEL_H
