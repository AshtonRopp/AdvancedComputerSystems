// sparse-sparse.h: Functions to perform sparse-sparse matrix multiplication

#ifndef SPARSE_SPARSE_H
#define SPARSE_SPARSE_H

class SparseMatrix; // Forward declaration

SparseMatrix multiplySparseMatrices_none(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols);

SparseMatrix multiplySparseMatrices_cache(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols, int blockSize);

void multiplyPartial(const SparseMatrix& A, const SparseMatrix& B, SparseMatrix& result, 
    int startRow, int endRow);

SparseMatrix multiplySparseMatrices_multithread(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols, int numThreads);

SparseMatrix multiplySparseMatrices_SIMD(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols);

void multiplyBlock(const SparseMatrix& A, const SparseMatrix& B, SparseMatrix& result, 
    int ii, int jj, int kk, int blockSize, int resultCols);

SparseMatrix multiplySparseMatrices_all(
    const SparseMatrix& A, const SparseMatrix& B, int resultRows, int resultCols,
    int blockSize, int numThreads);

#endif // SPARSE_SPARSE_H
