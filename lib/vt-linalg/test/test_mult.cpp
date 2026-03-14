#include <vt_linalg>
#include <iostream>
#include <cassert>

using namespace vt;

int main() {
    numeric_matrix<4, 3> A({{1, 2, 3},
                            {4, 5, 6},
                            {7, 8, 9},
                            {10, 11, 12}});

    numeric_matrix<4, 9> B({{1, 2, 3, 4, 5, 6, 7, 8, 9},
                            {1, 2, 3, 4, 5, 6, 7, 8, 9},
                            {1, 2, 3, 4, 5, 6, 7, 8, 9},
                            {1, 2, 3, 4, 5, 6, 7, 8, 9}});

    assert(A.transpose().matmul(B) == A.T_matmul(B));  // ATB
    assert(B.transpose().matmul(A) == B.T_matmul(A));  // BTA

    assert(A.matmul(A.transpose()) == A.matmul_T(A));  // AAT
    assert(A.transpose().matmul(A) == A.T_matmul(A));  // ATA

    assert(B.matmul(B.transpose()) == B.matmul_T(B));  //BBT
    assert(B.transpose().matmul(B) == B.T_matmul(B));  // BTB
    return 0;
}
