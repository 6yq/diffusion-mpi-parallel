#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DiffusionSolver.hpp"
#include "scenarios.hpp"

typedef struct {
  int n_theta, n_phi;
  double dt, radius, mu_a, mu_s;
  int num_iter, save_iter;
} Header;

int main(int argc, char **argv) {
  int n_theta = 120, n_phi = 240;
  int num_iter = 6000, save_iter = 500;
  double radius = 17.7, dt = 0.002, mu_a = 0.01, mu_s = 0.05;
  double ratio = 0.0;
  bool gen_ref = false;
  char output_file[256] = "";

  int cur_arg = 1;
  while (cur_arg < argc) {
    if (strcmp(argv[cur_arg], "--genRef") == 0) {
      gen_ref = true;
      cur_arg += 1;
      continue;
    }

    if (cur_arg + 1 >= argc) {
      fprintf(stderr, "Missing argument value for %s\n", argv[cur_arg]);
      return 1;
    }

    if (strcmp(argv[cur_arg], "--n-theta") == 0) {
      n_theta = atoi(argv[cur_arg + 1]);
    } else if (strcmp(argv[cur_arg], "--n-phi") == 0) {
      n_phi = atoi(argv[cur_arg + 1]);
    } else if (strcmp(argv[cur_arg], "--ratio") == 0) {
      ratio = atof(argv[cur_arg + 1]);
    } else {
      fprintf(stderr, "Unknown argument: %s\n", argv[cur_arg]);
      return 1;
    }

    cur_arg += 2;
  }

  double *phi = (double *)calloc(n_theta * n_phi, sizeof(double));
  if (!phi) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }

  generate_point_source(phi, n_theta, n_phi, ratio);

  clock_t init_start = clock();

  double D = 1.0 / (3.0 * (mu_a + mu_s));
  init(phi, n_theta, n_phi, radius, dt, mu_a, D);

  clock_t init_end = clock();
  printf("Initialization time: %fs\n",
         (double)(init_end - init_start) / CLOCKS_PER_SEC);

  if (gen_ref)
    sprintf(output_file, "ref/base.out");
  else
    sprintf(output_file, "output.out");

  FILE *fptr = fopen(output_file, "wb");
  if (!fptr) {
    fprintf(stderr, "Error opening file: %s\n", output_file);
    return 1;
  }

  Header h = {n_theta, n_phi, dt, radius, mu_a, mu_s, num_iter, save_iter};
  fwrite(&h, sizeof(Header), 1, fptr);

  clock_t start = clock();

  for (int t = 0; t < num_iter; t++) {
    if (t % save_iter == 0) {
      fwrite(phi, sizeof(double), n_theta * n_phi, fptr);
    }

    step();
  }

  clock_t end = clock();
  printf("Execution time: %fs\n", (double)(end - start) / CLOCKS_PER_SEC);

  clock_t free_start = clock();
  free_memory();
  clock_t free_end = clock();
  printf("Free memory time: %fs\n",
         (double)(free_end - free_start) / CLOCKS_PER_SEC);

  free(phi);
  fclose(fptr);

  printf("Output written to %s\n", output_file);
  return 0;
}
