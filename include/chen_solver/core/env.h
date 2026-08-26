/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Description: Solver environment holding global parameters (similar to GRBEnv in Gurobi).
 */

#ifndef CHEN_SOLVER_ENV_H
#define CHEN_SOLVER_ENV_H

#include "chen_solver/config.h"

namespace chen_solver
{
    // Holds solver-wide parameters. Construct one ChenEnv, set parameters,
    // then pass it to ChenModel. Pattern mirrors Gurobi's GRBEnv / GRBModel.
    class ChenEnv
    {
        double time_limit_     = INF;   // max solve time in seconds
        int    threads_        = 0;     // 0 = automatic (use all available cores)
        int    output_flag_    = 1;     // 1 = print solver log, 0 = silent
        double mip_gap_        = 1e-4;  // relative MIP optimality gap
        double feasibility_tol_ = 1e-6; // primal feasibility tolerance
        double int_feas_tol_   = 1e-5;  // integer feasibility tolerance

    public:
        ChenEnv() = default;

        // --- setters ---
        ChenEnv& setTimeLimit(const double seconds)      { time_limit_      = seconds; return *this; }
        ChenEnv& setThreads(const int n)                 { threads_         = n;       return *this; }
        ChenEnv& setOutputFlag(const int flag)           { output_flag_     = flag;    return *this; }
        ChenEnv& setMipGap(const double gap)             { mip_gap_         = gap;     return *this; }
        ChenEnv& setFeasibilityTol(const double tol)     { feasibility_tol_ = tol;     return *this; }
        ChenEnv& setIntFeasTol(const double tol)         { int_feas_tol_    = tol;     return *this; }

        // --- getters ---
        [[nodiscard]] double timeLimit()      const noexcept { return time_limit_;      }
        [[nodiscard]] int    threads()        const noexcept { return threads_;         }
        [[nodiscard]] int    outputFlag()     const noexcept { return output_flag_;     }
        [[nodiscard]] double mipGap()         const noexcept { return mip_gap_;         }
        [[nodiscard]] double feasibilityTol() const noexcept { return feasibility_tol_; }
        [[nodiscard]] double intFeasTol()     const noexcept { return int_feas_tol_;    }
    };

} // namespace chen_solver

#endif // CHEN_SOLVER_ENV_H
