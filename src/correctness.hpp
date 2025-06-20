#pragma once
#include <stdint.h>

typedef struct {
  int n_theta, n_phi;
  double dt, radius, mu_a, mu_s;
  int num_iter, save_iter;
} Header;

typedef struct {
  double ***data; // [frame][i][j]
  int num_frames;
  int n_theta;
  int n_phi;
} Dataset;

void read_dataset(const char *filename, Dataset *dataset, Header *header);

void free_dataset(Dataset *dataset);

double compute_max_error(const Dataset *d1, const Dataset *d2);

void generate_error_curve(const Dataset *d1, const Dataset *d2,
                          const Header *h);

void print_error(const char *output_filename, int n_theta);
