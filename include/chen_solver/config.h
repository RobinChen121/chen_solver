#ifndef CHEN_SOLVER_CONFIG_H
#define CHEN_SOLVER_CONFIG_H


// 将许多代码放到 namespace里面是 C++ 库项目的常规做法，核心目的是避免命名冲突和明确 API 归属。
// 项目会定义很多通用名字（Model, Variable, status, INF），如果不放进 namespace chen_solver，
// 很容易和别的库或业务代码重名。放进命名空间后，用户通过 chen_solver::Model 使用，边界清晰，也更适合后续做工业级扩展
// （多模块、多后端、多第三方依赖）

namespace chen_solver {

using ChenInt = int32_t;
using VarId = uint64_t;
using ConId = uint64_t;


// constexpr 表示变量不仅不能修改，并且在编译时就确定的值
constexpr double EPS = 1e-9;
constexpr double INF = 1e100;

enum class ObjSense : uint8_t {
    Minimize, // Minimize the objective function
    Maximize,
};

enum class VarType : uint8_t { Continuous, Integer, Binary };

enum class ModelStatus : uint8_t {
    Empty,
    Modified,
    Compiled,
};

} // namespace chen_solver

#endif //CHEN_SOLVER_CONFIG_H
