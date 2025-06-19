#include "scenarios.hpp"
#include "note.hpp"
#include <cmath>

void generate_point_source(double *phi, int N_THETA, int N_PHI, double ratio) {
  int i0 = static_cast<int>(ratio * (N_THETA - 1));
  // put a point-like source at the mid of the longitude
  int j0 = N_PHI / 2;

  for (int i = 0; i < N_THETA; ++i)
    for (int j = 0; j < N_PHI; ++j)
      phi(i, j) = 0.0;

  phi(i0, j0) = 1.0;
}
