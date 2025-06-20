#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DiffusionSolver.hpp"
#include "correctness.hpp"
#include "scenarios.hpp"

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

  int rank, num_procs;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  double *phi = NULL;
  if (rank == 0) {
    phi = (double *)calloc(n_theta * n_phi, sizeof(double));
    if (!phi) {
      fprintf(stderr, "Memory allocation failed\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    generate_point_source(phi, n_theta, n_phi, ratio);
  }

  clock_t init_start = clock();
  double D = 1.0 / (3.0 * (mu_a + mu_s));
  init(phi, n_theta, n_phi, radius, dt, mu_a, D);
  clock_t init_end = clock();

  if (rank == 0)
    printf("Initialization time: %fs\n",
           (double)(init_end - init_start) / CLOCKS_PER_SEC);

  if (gen_ref)
    sprintf(output_file, "ref/base.out");
  else
    sprintf(output_file, "opt/opt.out");

  FILE *fptr = NULL;
  if (rank == 0) {
    fptr = fopen(output_file, "wb");
    if (!fptr) {
      fprintf(stderr, "Error opening file: %s\n", output_file);
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    Header h = {n_theta, n_phi, dt, radius, mu_a, mu_s, num_iter, save_iter};
    fwrite(&h, sizeof(Header), 1, fptr);
  }

  double *phi_gathered = NULL;
  if (rank == 0)
    phi_gathered = (double *)calloc(n_theta * n_phi, sizeof(double));

  clock_t start = clock();
  for (int t = 0; t < num_iter; t++) {
    if (t % save_iter == 0) {
      transfer(phi_gathered);
      if (rank == 0)
        fwrite(phi_gathered, sizeof(double), n_theta * n_phi, fptr);
    }
    step();
  }
  clock_t end = clock();

  if (rank == 0)
    printf("Execution time: %fs\n", (double)(end - start) / CLOCKS_PER_SEC);

  clock_t free_start = clock();
  free_memory();
  clock_t free_end = clock();

  if (rank == 0) {
    printf("Free memory time: %fs\n",
           (double)(free_end - free_start) / CLOCKS_PER_SEC);
    free(phi);
    free(phi_gathered);
    fclose(fptr);
    printf("Output written to %s\n", output_file);
    if (!gen_ref) {
      print_error(output_file, n_theta);
    }
  }

  MPI_Finalize();
  return 0;
}
