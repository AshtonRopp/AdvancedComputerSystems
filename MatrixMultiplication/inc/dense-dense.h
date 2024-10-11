// dense-dense.h: Functions to perform dense-dense matrix multiplication

#ifndef DENSE_DENSE_H
#define DENSE_DENSE_H

#include<vector>

std::vector<std::vector<int>> multiplyDenseMatrices_none(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B);

std::vector<std::vector<int>> multiplyDenseMatrices_cache(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int blockSize);

void multiplyRowRange(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, std::vector<std::vector<int>>& result, int startRow, int endRow);

std::vector<std::vector<int>> multiplyDenseMatrices_multithread(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int numThreads);

std::vector<std::vector<int>> multiplyDenseMatrices_SIMD(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B);

std::vector<std::vector<int>> multiplyDenseMatrices_all(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, int blockSize, int numThreads);

#endif // DENSE_DENSE_H