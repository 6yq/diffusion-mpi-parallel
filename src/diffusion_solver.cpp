#include "diffusion_solver.hpp"
#include "note.hpp"
#include <cmath>
#include <cstdlib>

int N_THETA, N_PHI;
double R, dt, mu_a, D, dtheta, dphi;
double *phi, *rhs, *rhs1, *rhs2;
int t = 0;

void init(double *phi0, int n_theta, int n_phi, double radius_, double dt_,
          double mu_a_, double D_) {
  phi = phi0;
  N_THETA = n_theta;
  N_PHI = n_phi;
  R = radius_;
  dt = dt_;
  mu_a = mu_a_;
  D = D_;
  dtheta = M_PI / (N_THETA - 1);
  dphi = 2 * M_PI / N_PHI;

  rhs = (double *)calloc(N_THETA * N_PHI, sizeof(double));
  rhs1 = (double *)calloc(N_THETA * N_PHI, sizeof(double));
  rhs2 = (double *)calloc(N_THETA * N_PHI, sizeof(double));
}

/**
 * compute derivs
 */
void compute_rhs() {
  for (int i = 1; i < N_THETA - 1; i++) {
    double theta = i * dtheta;
    double sin_theta = sin(theta);
    for (int j = 0; j < N_PHI; j++) {
      int jm1 = (j - 1 + N_PHI) % N_PHI;
      int jp1 = (j + 1) % N_PHI;

      double d2theta =
          (phi(i + 1, j) - 2 * phi(i, j) + phi(i - 1, j)) / (dtheta * dtheta);
      double dtheta_term = cos(theta) / sin_theta *
                           (phi(i + 1, j) - phi(i - 1, j)) / (2 * dtheta);
      double d2phi =
          (phi(i, jp1) - 2 * phi(i, j) + phi(i, jm1)) / (dphi * dphi);
      double lap = (1.0 / (R * R)) *
                   (d2theta + dtheta_term + d2phi / (sin_theta * sin_theta));

      rhs(i, j) = D * lap - mu_a * phi(i, j);
    }
  }

  // Neumann boundary condition
  for (int j = 0; j < N_PHI; j++) {
    rhs(0, j) = rhs(1, j);
    rhs(N_THETA - 1, j) = rhs(N_THETA - 2, j);
  }
}

/**
 * compute the next time step using multistep method
 */
void multistep(double a1, double a2, double a3) {
  for (int i = 1; i < N_THETA - 1; i++) {
    for (int j = 0; j < N_PHI; j++) {
      phi(i, j) += dt * (a1 * rhs(i, j) + a2 * rhs1(i, j) + a3 * rhs2(i, j));
    }
  }
}

/**
 * step fm
 * Also use Adams-Bashforth
 */
void step() {
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

  // swap buffers for multistep method
  double *tmp = rhs2;
  rhs2 = rhs1;
  rhs1 = rhs;
  rhs = tmp;

  t++;
}

void free_memory() {
  free(rhs);
  free(rhs1);
  free(rhs2);
}
