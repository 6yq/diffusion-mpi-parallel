#pragma once

void init(double *phi0, int n_theta, int n_phi, double radius, double dt,
          double mu_a, double D);

void step();

void free_memory();
