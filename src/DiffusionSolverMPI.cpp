#include "DiffusionSolver.hpp"
#include "note.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <mpi.h>

int N_THETA, N_PHI;
double R, dt, mu_a, D, dtheta, dphi;
double *phi_local, *rhs_local, *rhs1_local, *rhs2_local;
int t = 0;
int rank, num_procs;
int local_start, local_end, local_size;

int *gather_recvcounts = nullptr;
int *gather_displs = nullptr;

void exchange_ghost_cells(double *phi) {
  MPI_Status status;
  int up = rank - 1;
  int down = rank + 1;

  // send up, receive from down
  if (down < num_procs) {
    MPI_Sendrecv(&phi[(local_size - 2) * N_PHI], N_PHI, MPI_DOUBLE, down, 0,
                 &phi[(local_size - 1) * N_PHI], N_PHI, MPI_DOUBLE, down, 1,
                 MPI_COMM_WORLD, &status);
  }

  // send down, receive from up
  if (up >= 0) {
    MPI_Sendrecv(&phi[1 * N_PHI], N_PHI, MPI_DOUBLE, up, 1, &phi[0 * N_PHI],
                 N_PHI, MPI_DOUBLE, up, 0, MPI_COMM_WORLD, &status);
  }
}

void init(double *phi_global, int n_theta, int n_phi, double radius_,
          double dt_, double mu_a_, double D_) {
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  N_THETA = n_theta;
  N_PHI = n_phi;
  R = radius_;
  dt = dt_;
  mu_a = mu_a_;
  D = D_;
  dtheta = M_PI / (N_THETA - 1);
  dphi = 2 * M_PI / N_PHI;

  // Compute distribution
  int *sendcounts = (int *)malloc(num_procs * sizeof(int));
  int *displs = (int *)malloc(num_procs * sizeof(int));
  int offset = 0;

  for (int i = 0; i < num_procs; ++i) {
    int rows = (N_THETA - 2) / num_procs + (i < (N_THETA - 2) % num_procs);
    sendcounts[i] = rows * N_PHI; // number of elements
    displs[i] = offset;
    offset += sendcounts[i];
  }

  gather_recvcounts = (int *)malloc(num_procs * sizeof(int));
  gather_displs = (int *)malloc(num_procs * sizeof(int));

  for (int i = 0; i < num_procs; ++i) {
    int rows = (N_THETA - 2) / num_procs + (i < (N_THETA - 2) % num_procs);
    gather_recvcounts[i] = rows * N_PHI;
  }

  gather_displs[0] = N_PHI;
  for (int i = 1; i < num_procs; ++i) {
    gather_displs[i] = gather_displs[i - 1] + gather_recvcounts[i - 1];
  }

  // Compute local bounds
  int chunk = (N_THETA - 2) / num_procs;
  int rem = (N_THETA - 2) % num_procs;

  local_start = 1 + rank * chunk + std::min(rank, rem);
  local_end = local_start + chunk - 1 + (rank < rem ? 1 : 0);
  local_size = local_end - local_start + 3; // Include top and bottom halo rows

  phi_local = (double *)calloc(local_size * N_PHI, sizeof(double));
  rhs_local = (double *)calloc(local_size * N_PHI, sizeof(double));
  rhs1_local = (double *)calloc(local_size * N_PHI, sizeof(double));
  rhs2_local = (double *)calloc(local_size * N_PHI, sizeof(double));

  MPI_Scatterv(rank == 0 ? (phi_global + N_PHI) : NULL, sendcounts, displs,
               MPI_DOUBLE, phi_local + N_PHI, sendcounts[rank], MPI_DOUBLE, 0,
               MPI_COMM_WORLD);

  free(sendcounts);
  free(displs);
}

void compute_rhs() {
  // Loop over local inner grid points (excluding halo)
  for (int i = 1; i < local_size - 1; i++) {
    double theta = (local_start + i - 1) * dtheta; // global theta index
    double sin_theta = sin(theta);
    for (int j = 0; j < N_PHI; j++) {
      int jm1 = (j - 1 + N_PHI) % N_PHI; // periodic left
      int jp1 = (j + 1) % N_PHI;         // periodic right

      // Second derivative in theta direction
      double d2theta =
          (phi_local(i + 1, j) - 2 * phi_local(i, j) + phi_local(i - 1, j)) /
          (dtheta * dtheta);

      // First derivative term due to spherical coordinates
      double dtheta_term = cos(theta) / sin_theta *
                           (phi_local(i + 1, j) - phi_local(i - 1, j)) /
                           (2 * dtheta);

      // Second derivative in phi direction
      double d2phi =
          (phi_local(i, jp1) - 2 * phi_local(i, j) + phi_local(i, jm1)) /
          (dphi * dphi);

      // Combine Laplacian terms in spherical coordinates
      double lap = (1.0 / (R * R)) *
                   (d2theta + dtheta_term + d2phi / (sin_theta * sin_theta));

      // Update RHS
      rhs_local(i, j) = D * lap - mu_a * phi_local(i, j);
    }
  }

  // Apply Neumann boundary condition in phi (ends of theta handled by ghost
  // cells)
  if (rank == 0) {
    for (int j = 0; j < N_PHI; j++)
      rhs_local(1, j) = rhs_local(2, j); // bottom ghost
  }
  if (rank == num_procs - 1) {
    for (int j = 0; j < N_PHI; j++)
      rhs_local(local_size - 2, j) = rhs_local(local_size - 3, j); // top ghost
  }
}

void multistep(double a1, double a2, double a3) {
  for (int i = 1; i < local_size - 1; i++) {
    for (int j = 0; j < N_PHI; j++) {
      phi_local(i, j) += dt * (a1 * rhs_local(i, j) + a2 * rhs1_local(i, j) +
                               a3 * rhs2_local(i, j));
    }
  }
}

void step() {
  exchange_ghost_cells(phi_local);
  compute_rhs();

  double a1, a2, a3;
  if (t == 0) {
    a1 = 1.0;
    a2 = 0.0;
    a3 = 0.0;
  } else if (t == 1) {
    a1 = 3.0 / 2.0;
    a2 = -1.0 / 2.0;
    a3 = 0.0;
  } else {
    a1 = 23.0 / 12.0;
    a2 = -16.0 / 12.0;
    a3 = 5.0 / 12.0;
  }
  multistep(a1, a2, a3);

  double *tmp = rhs2_local;
  rhs2_local = rhs1_local;
  rhs1_local = rhs_local;
  rhs_local = tmp;

  t++;
}

void transfer(double *phi_global) {
  MPI_Gatherv(&phi_local[1 * N_PHI], (local_size - 2) * N_PHI, MPI_DOUBLE,
              rank == 0 ? (phi_global + N_PHI) : NULL, gather_recvcounts,
              gather_displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void free_memory() {
  free(phi_local);
  free(rhs_local);
  free(rhs1_local);
  free(rhs2_local);

  if (gather_recvcounts)
    free(gather_recvcounts);
  if (gather_displs)
    free(gather_displs);
}