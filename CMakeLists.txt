cmake_minimum_required(VERSION 3.20) # 指定 CMake 最低版本要求，避免低版本语法不兼容
project(chen_solver VERSION 0.1.0 LANGUAGES CXX) # 定义项目名、版本和启用的语言（C++）

set(CMAKE_CXX_STANDARD 20) # 全局设置 C++ 标准为 C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 要求编译器必须支持 C++20，禁止自动降级
set(CMAKE_CXX_EXTENSIONS OFF) # 关闭编译器扩展（如 gnu++20），保持跨平台一致性

if(MSVC) # 判断是否使用 MSVC 编译器
    set(CMAKE_CXX_FLAGS_RELEASE "/O2") # MSVC Release 模式启用 /O2 优化
else() # 非 MSVC 分支（如 Clang/GCC）
    set(CMAKE_CXX_FLAGS_RELEASE "-O3") # 非 MSVC Release 模式启用 -O3 优化
endif() # 结束编译器分支判断

add_library(chen_solver_core # 创建核心静态库目标 chen_solver_core，承载求解器公共能力
        src/core/model.cpp # 将模型核心实现编译进库
) # 结束库目标源文件列表
target_include_directories(chen_solver_core # 为 chen_solver_core 设置头文件搜索路径
        PUBLIC # PUBLIC 表示库自身和依赖方都可见该头文件目录
        ${CMAKE_CURRENT_SOURCE_DIR}/include # 对外公开 include/ 作为主头文件根目录
) # 结束 include 目录配置
target_compile_features(chen_solver_core PUBLIC cxx_std_20) # 显式声明该库对外要求 C++20 特性

add_executable(chen_solver # 创建命令行可执行目标 chen_solver（可直接运行 main.cpp）
        main.cpp # CLI 入口文件
) # 结束 CLI 可执行目标源文件列表
target_link_libraries(chen_solver PRIVATE chen_solver_core) # 可执行程序私有链接核心库，不向外传播链接依赖

option(CHEN_SOLVER_BUILD_EXAMPLES "Build example programs" ON) # 提供开关控制是否构建 examples
if(CHEN_SOLVER_BUILD_EXAMPLES) # 当开关开启时构建示例程序
    add_executable(chen_solver_basic_example # 创建基础示例可执行目标
            examples/basic_model.cpp # 示例入口源文件
    ) # 结束示例可执行目标源文件列表
    target_link_libraries(chen_solver_basic_example PRIVATE chen_solver_core) # 示例程序私有链接核心库
endif() # 结束示例构建条件

include(CTest) # 启用 CTest 测试框架支持（提供 BUILD_TESTING 选项）
if(BUILD_TESTING) # 当测试开关开启时构建并注册测试
    add_executable(chen_solver_model_smoke_test # 创建轻量 smoke test 可执行目标
            tests/model_smoke_test.cpp # smoke test 源文件
    ) # 结束测试可执行目标源文件列表
    target_link_libraries(chen_solver_model_smoke_test PRIVATE chen_solver_core) # 测试程序私有链接核心库
    add_test(NAME model_smoke_test COMMAND chen_solver_model_smoke_test) # 向 CTest 注册测试用例
endif() # 结束测试构建条件
