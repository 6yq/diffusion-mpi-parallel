#pragma once

void init(int n_theta, int n_phi, double radius_, double dt_, double mu_a_,
          double D_);
void step(double **phi);
void free_memory();
