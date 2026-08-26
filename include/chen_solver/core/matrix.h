/**
 * Author: Zhen Chen
 * Email: chen.zhen5526@gmail.com
 * Created on: 2026/08/20, 17:01
 * Description: Compressed Sparse format for sparse matrix representation:
 * CSC (Compressed Sparse Column) format  or CSR (Compressed Sparse Row)
 * is a widely used data structure for efficiently storing and manipulating sparse matrices.
 * In this format (CSC), the non-zero elements of the matrix are stored in a one-dimensional array,
 * along with two additional arrays that provide information about the row indices
 * and column pointers of the non-zero elements.
 * This allows for efficient access to the non-zero elements and their corresponding row and column indices,
 * making it suitable for various numerical computations involving sparse matrices.
 *
 * 
 */


#ifndef CHEN_SOLVER_MATRIX_H
#define CHEN_SOLVER_MATRIX_H

#include <vector>
#include "chen_solver/config.h"

// 一个例子 [[0, 0, 3, 0], [5, 0, 0, 0], [0, 0, 0, 2], [0, 8, 0, 1]]
// 0x_1 + 0x_2 + 3x_3 + 0x_4
// 5x_1 + 0x_2 + 0x_3 + 0x_4
// 0x_1 + 0x_2 + 0x_3 + 2x_4
// 0x_1 + 8x_2 + 0x_3 + 1x_4
// values = {5, 8, 3, 2, 1}
// row_indices = {1, 3, 0, 2, 3}
// col_ptr = {0, 1, 2, 3, 5}, col[i+1]-col[i] equals the number of non-zeros in column i

    struct CSC {
        std::vector<double> values{}; // non zeros values
        std::vector<ChenInt> row_indices{}; // row indices for the non-zero values
        // col_ptr 的个数为列数加 1
        // 记录每一列的第一个非零元素在 values 数组中的起始位置（索引）
        std::vector<ChenInt> col_ptr{};
        // start and end indices in the non-zero values in each column

        CSC() = default; // 空对象方便临时使用
        // num_non_zero = 预计非零元素个数
        // num_col = 矩阵列数
        // 工业级时最好估计非零个数，从而提前分配内存
        // array 也可以直接用数值初始化，此时元素都为 0
        CSC(const ChenInt num_non_zero, const ChenInt num_col)
            : values(num_non_zero), row_indices(num_non_zero), col_ptr(num_col + 1) {
        }
    };
#endif //CHEN_SOLVER_MATRIX_H
