/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 13:03
 * Description: 
 *
 */

#ifndef CHEN_SOLVER_CONFIG_H
#define CHEN_SOLVER_CONFIG_H

#include <cstdint>

// 将许多代码放到 namespace里面是 C++ 库项目的常规做法，核心目的是避免命名冲突和明确 API 归属。
// 项目会定义很多通用名字（Model, Variable, status, INF），如果不放进 namespace chen_solver，
// 很容易和别的库或业务代码重名。放进命名空间后，用户通过 chen_solver::Model 使用，边界清晰，也更适合后续做工业级扩展
// （多模块、多后端、多第三方依赖）

namespace chen_solver {
    // int8_t 的取值范围是 -128 到 127（即 -2^{7}$ 到 2^{7} - 1)，占用1个字节 (8 bits)
    // uint8_t 的取值范围是 0 到 255（即 0 到 2^{8} - 1)，占用1个字节 (8 bits)
    // int16_t 的取值范围是 -32,768 到 32,767（即 -2^{15}$ 到 2^{15} - 1)，占用2个字节 (16 bits)
    // int32_t 的取值范围是 -2,147,483,648 到 2,147,483,647（即 -2^{31}$ 到 2^{31} - 1)，占用4个字节 (32 bits)
    // int64_t 的取值范围是 -9,223,372,036,854,775,808 到 9,223,372,036,854,775,807（即 -2^{63}$ 到 2^{63} - 1)，占用8个字节 (64 bits)
    // uint64_t 的取值范围是 0 到 18,446,744,073,709,551,615（即 0 到 2^{64} - 1)，占用8个字节 (64 bits)
    // 在现代常见的 32 位和 64 位操作系统（如 Windows、Linux、macOS）上，int 均为 4 字节（32 位）
    // 但在某些嵌入式系统或特定编译器下，int 可能为 2 字节（16 位）或 8 字节（64 位），因此不建议直接使用 int
    // 如果代码中对数据位宽或跨平台兼容性有严格要求，应优先使用 <cstdint> 标头中定义的显式位宽类型（如 int32_t、int64_t），避免直接使用 int。
    // uint32_t 的取值范围是 0 到 4,294,967,295（即 0 到 2^{32} - 1, 42.9亿)，占用4个字节 (32 bits)
    using ChenInt = int64_t;
    using ChenUInt = uint64_t;

    // double 的取值范围是 2.2250738585072014e-308 到 1.7976931348623157e+308，占用8个字节 (64 bits)
    // constexpr 表示变量不仅不能修改，并且在编译时就确定的值
    constexpr double EPS = 1e-9;
    constexpr double INF = 1e100;

    // enum class 若不指定底层类型，通常默认是 int，占用4个字节 (32 bits)。
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
