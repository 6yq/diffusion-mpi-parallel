#include "correctness.hpp"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read_dataset(const char *filename, Dataset *dataset, Header *header) {
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Error opening file: %s\n", filename);
    exit(EXIT_FAILURE);
  }

  if (fread(header, sizeof(Header), 1, fp) != 1) {
    fprintf(stderr, "Error reading header from%s\n", filename);
    exit(EXIT_FAILURE);
  }

  int n_theta = header->n_theta;
  int n_phi = header->n_phi;
  int num_frames =
      (header->num_iter + header->save_iter - 1) / header->save_iter;

  double ***data = (double ***)malloc(num_frames * sizeof(double **));
  for (int f = 0; f < num_frames; ++f) {
    data[f] = (double **)malloc(n_theta * sizeof(double *));
    for (int i = 0; i < n_theta; ++i) {
      data[f][i] = (double *)malloc(n_phi * sizeof(double));
      if (fread(data[f][i], sizeof(double), n_phi, fp) != (size_t)n_phi) {
        fprintf(stderr, "Error reading data frame %d\n", f);
        exit(EXIT_FAILURE);
      }
    }
  }

  dataset->data = data;
  dataset->num_frames = num_frames;
  dataset->n_theta = n_theta;
  dataset->n_phi = n_phi;

  fclose(fp);
}

void free_dataset(Dataset *dataset) {
  for (int f = 0; f < dataset->num_frames; ++f) {
    for (int i = 0; i < dataset->n_theta; ++i) {
      free(dataset->data[f][i]);
    }
    free(dataset->data[f]);
  }
  free(dataset->data);
}

double compute_max_error(const Dataset *d1, const Dataset *d2) {
  double max_error = 0.0;
  for (int f = 0; f < d1->num_frames; ++f) {
    for (int i = 0; i < d1->n_theta; ++i) {
      for (int j = 0; j < d1->n_phi; ++j) {
        double err = fabs(d1->data[f][i][j] - d2->data[f][i][j]);
        if (err > max_error)
          max_error = err;
      }
    }
  }
  return max_error;
}

void generate_error_curve(const Dataset *d1, const Dataset *d2,
                          const Header *h) {
  for (int f = 0; f < d1->num_frames; ++f) {
    double frame_max = 0.0;
    for (int i = 0; i < d1->n_theta; ++i) {
      for (int j = 0; j < d1->n_phi; ++j) {
        double err = fabs(d1->data[f][i][j] - d2->data[f][i][j]);
        if (err > frame_max)
          frame_max = err;
      }
    }
    printf("iter=%d,error=%.15e\n", f * h->save_iter, frame_max);
  }
}

void print_error(const char *output_filename, int n_theta) {
  char ref_filename[256];
  snprintf(ref_filename, sizeof(ref_filename), "ref/base%d.out", n_theta);

  Header h1, h2;
  Dataset ds1, ds2;

  read_dataset(ref_filename, &ds1, &h1);
  read_dataset(output_filename, &ds2, &h2);

  if (memcmp(&h1, &h2, sizeof(Header)) != 0) {
    fprintf(stderr, "Header mismatch!\n");
    exit(EXIT_FAILURE);
  }

  double max_err = compute_max_error(&ds1, &ds2);
  if (max_err > 10e-14) {
    printf("Significant numeric error: %.15e\n", max_err);
    exit(EXIT_FAILURE);
  } else {
    printf("Max error: %.15e\n", max_err);
  }

  free_dataset(&ds1);
  free_dataset(&ds2);
}
