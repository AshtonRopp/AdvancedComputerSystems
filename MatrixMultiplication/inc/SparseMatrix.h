// SparseMatrix.h: Holds sparse matrix struct

#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

#include <vector>

// Structure to represent a sparse matrix in LIL format
struct SparseMatrix {
    std::vector<std::vector<int>> rows;   // Row-wise storage
    std::vector<std::vector<int>> values; // Values corresponding to the indices
};

#endif // SPARSE_MATRIX_H