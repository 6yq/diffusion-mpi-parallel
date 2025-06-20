#pragma once

#define phi(i, j) phi[(i) * N_PHI + (j)]
#define rhs(i, j) rhs[(i) * N_PHI + (j)]
#define rhs1(i, j) rhs1[(i) * N_PHI + (j)]
#define rhs2(i, j) rhs2[(i) * N_PHI + (j)]

// for mpi
#define phi_local(i, j) phi_local[(i) * N_PHI + (j)]
#define rhs_local(i, j) rhs_local[(i) * N_PHI + (j)]
#define rhs1_local(i, j) rhs1_local[(i) * N_PHI + (j)]
#define rhs2_local(i, j) rhs2_local[(i) * N_PHI + (j)]
