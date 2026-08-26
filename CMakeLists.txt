cmake_minimum_required(VERSION 3.20) # 指定 CMake 最低版本要求，避免低版本语法不兼容
project(chen_solver VERSION 1.0.0 LANGUAGES CXX) # 定义项目名、版本号和启用的语言（C++）

include(GNUInstallDirs) # 引入标准安装目录变量，如 bin、lib、include

set(CMAKE_CXX_STANDARD 20) # 全局设置 C++ 标准为 C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 要求编译器必须支持 C++20，禁止自动降级
set(CMAKE_CXX_EXTENSIONS OFF) # 关闭编译器扩展（如 gnu++20），保持跨平台一致性

# cmake option() 命令用于定义用户可配置的开关选项，提供默认值和描述信息
# 它第一个参数为选项名，最终会是一个 CMake 变量，第二个参数为描述信息，第三个参数为默认值（ON/OFF）
option(CHEN_SOLVER_BUILD_EXAMPLES "Build example programs" ON) # 提供开关控制是否构建 examples
# 开启编译器警告意味着编译器会更严格地提示例如：未使用变量 / 未使用参数；有符号和无符号比较；可能有问题的类型转换；
#不太标准或不够规范的写法；某些潜在 bug
option(CHEN_SOLVER_ENABLE_WARNINGS "Enable compiler warnings" ON) # 提供开关控制是否启用编译警告

# cmake 中的函数定义语法为 function(<name> [arg1 [arg2 ...]] <body> endfunction())
# 小括号中第一个为函数名，之后为参数列表，函数体中可以使用 ${arg1}、${arg2} 等访问参数值
function(chen_solver_enable_warnings target_name) # 定义一个辅助函数，为指定目标统一开启编译警告
    if (NOT CHEN_SOLVER_ENABLE_WARNINGS) # 如果用户关闭了警告开关，则直接跳过
        return() # 提前返回，不对目标添加任何警告选项
    endif () # 结束警告开关判断

    if (MSVC) # 如果当前编译器是 MSVC
        target_compile_options(${target_name} PRIVATE /W4 /permissive- /utf-8) # 开启高等级警告、严格标准兼容和 UTF-8 源码支持
    else () # 非 MSVC 编译器（如 GCC/Clang）
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic) # 开启常用严格警告选项
    endif () # 结束编译器分支判断
endfunction() # 结束警告辅助函数定义

# add_library() 命令用于创建一个库目标，支持静态库、动态库和接口库
# 它的语法是 add_library(<name> [STATIC | SHARED | MODULE | INTERFACE] [EXCLUDE_FROM_ALL] source1 source2 ... sourceN)
add_library(chen_solver_core # 创建核心静态库 chen_solver_core，承载求解器公共能力
        src/core/model.cpp # 将模型核心实现编译进库
        src/io/read_file.cpp # 将 LP/MPS 读取实现编译进库
        src/presolve/presolve.cpp # 将预处理实现编译进库
        src/util/logger.cpp # 将日志工具实现编译进库
) # 结束库目标源文件列表
target_compile_definitions(chen_solver_core
        PUBLIC CHEN_SOLVER_VERSION="${PROJECT_VERSION}" # 为核心库定义一个宏 CHEN_SOLVER_VERSION，值为项目版本号，供外部使用
)
add_library(chen_solver::core ALIAS chen_solver_core) # 为核心库创建命名空间别名，方便外部统一链接
# 下吗是设置搜索头文件 h 的路径，上面 add_library() 命令只会把 cpp 文件编译进库，但外部使用该库时还需要知道头文件在哪里
target_include_directories(chen_solver_core # 为 chen_solver_core 设置头文件搜索路径
        PUBLIC # PUBLIC 表示库自身和依赖方都可见该头文件目录
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> # 对外公开 include/ 作为主头文件根目录
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # 安装后从标准 include 目录查找公共头文件
) # 结束 include 目录配置
# 下吗这个命令表明外部库调用 chen_solver_core 时，必须使用 C++20 特性，否则会报错
target_compile_features(chen_solver_core PUBLIC cxx_std_20) # 显式声明该库对外要求 C++20 特性
chen_solver_enable_warnings(chen_solver_core) # 为核心库启用统一警告策略

add_executable(chen_solver # 创建命令行可执行目标 chen_solver（可直接运行 main.cpp）
        main.cpp # CLI 入口文件
) # 结束 CLI 可执行目标源文件列表
target_link_libraries(chen_solver PRIVATE chen_solver_core) # 可执行程序私有链接核心库，不向外传播链接依赖
chen_solver_enable_warnings(chen_solver) # 为 CLI 目标启用统一警告策略

if (CHEN_SOLVER_BUILD_EXAMPLES) # 当开关开启时构建示例程序
    add_executable(chen_solver_example # 创建基础示例可执行目标
            examples/basic_lp.cpp # 示例入口源文件
    ) # 结束示例可执行目标源文件列表
    target_link_libraries(chen_solver_example PRIVATE chen_solver_core) # 示例程序私有链接核心库
    chen_solver_enable_warnings(chen_solver_example) # 为示例目标启用统一警告策略
endif () # 结束示例构建条件

# 测试相关配置
# CTest 是 CMake 自带的测试框架，提供了 BUILD_TESTING 选项和 add_test() 命令
include(CTest) # 启用 CTest 测试框架支持（提供 BUILD_TESTING 选项）
if (BUILD_TESTING) # 当测试开关开启时构建并注册测试
    # 每个文件测试不同模块，所以拆开测试
    add_executable(chen_solver_model_test # 创建test 可执行目标
            tests/model_test.cpp # smoke test 源文件
    ) # 结束测试可执行目标源文件列表
    target_link_libraries(chen_solver_model_test PRIVATE chen_solver::core) # 测试程序私有链接核心库
    chen_solver_enable_warnings(chen_solver_model_test) # 为测试目标启用统一警告策略
    add_test(NAME model_test COMMAND chen_solver_model_test) # 向 CTest 注册测试用例

    add_executable(chen_solver_presolve_test
            tests/presolve_test.cpp
    )
    target_link_libraries(chen_solver_presolve_test PRIVATE chen_solver::core)
    chen_solver_enable_warnings(chen_solver_presolve_test)
    add_test(NAME presolve_test COMMAND chen_solver_presolve_test)

    add_executable(chen_solver_logger_test
            tests/logger_test.cpp
    )
    target_link_libraries(chen_solver_logger_test PRIVATE chen_solver::core)
    chen_solver_enable_warnings(chen_solver_logger_test)
    add_test(NAME logger_test COMMAND chen_solver_logger_test)

    add_executable(chen_solver_modeling_dsl_test
            tests/modeling_dsl_test.cpp
    )
    target_link_libraries(chen_solver_modeling_dsl_test PRIVATE chen_solver::core)
    chen_solver_enable_warnings(chen_solver_modeling_dsl_test)
    add_test(NAME modeling_dsl_test COMMAND chen_solver_modeling_dsl_test)
endif () # 结束测试构建条件

# install() 的作用是：把“构建产物”拷到一个安装目录，用于分发/部署/给别人使用
# 其他人先用 cmake --install . 命令把构建产物拷到指定目录，
# 前提是用 cmake -S . -B build/ 生成了构建文件，cmake --build build/ 编译了构建产物
# 然后在他们的 CMakeLists.txt 中使用 find_package() 找到这个包并链接使用

# 使用顺序为：cmake -S . -B build 配置项目
# cmake --build build 编译项目
# cmake --install build

# 有时候上述命令会报错，可能因为当前电脑有多个 clang (例如anaconda里也包含clang) 或 gcc 版本，导致 cmake 找不到正确的编译器
# 此时可以在命令行中指定编译器路径，即将正确的编译器路径放到 PATH 环境变量中前面的位置，例如：export PATH="/opt/homebrew/opt/llvm/bin:$PATH" hash -r

# 命令行 cmake -S . -B build/ 在当前目录（.）中读取 CMakeLists.txt，配置项目，并把生成的构建文件输出到 build/ 目录中
# 之后再执行： cmake --build build 才会开始编译
install(TARGETS chen_solver_core chen_solver # 安装核心库和 CLI 可执行文件
        EXPORT chen_solverTargets # 同时把这些目标加入导出集合，供下游项目导入
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # 可执行文件安装到标准 bin 目录
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # 动态库安装到标准 lib 目录
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}) # 静态库安装到标准 lib 目录

install(DIRECTORY include/ # 安装整个公共头文件目录
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) # 头文件安装到标准 include 目录

install(EXPORT chen_solverTargets # 安装前面导出的目标集合
        FILE chen_solverTargets.cmake # 导出文件名固定为 chen_solverTargets.cmake
        NAMESPACE chen_solver:: # 为导出目标附加 chen_solver:: 命名空间
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/chen_solver) # 导出配置安装到包配置常见目录
