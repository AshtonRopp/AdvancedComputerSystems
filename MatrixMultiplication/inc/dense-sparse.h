// dense-sparse.h: Functions to perform dense-sparse matrix multiplication

#ifndef DENSE_SPARSE_H
#define DENSE_SPARSE_H

#include <vector>

class SparseMatrix; // Forward declaration

std::vector<std::vector<int>> multiplyDenseSparseMatrices_none(
    const std::vector<std::vector<int>>& dense, const SparseMatrix& sparse);

std::vector<std::vector<int>> multiplyDenseSparseMatrices_cache(
    const std::vector<std::vector<int>>& dense, const SparseMatrix& sparse, int blockSize);

std::vector<std::vector<int>> multiplyDenseSparseMatrices_multithread(
    const std::vector<std::vector<int>>& dense, const SparseMatrix& sparse, int numThreads);

std::vector<std::vector<int>> multiplyDenseSparseMatrices_SIMD(
    const std::vector<std::vector<int>>& dense, const SparseMatrix& sparse);

std::vector<std::vector<int>> multiplyDenseSparseMatrices_all(
    const std::vector<std::vector<int>>& dense, const SparseMatrix& sparse, int blockSize, int numThreads);

#endif // DENSE_SPARSE_H
