#include <stdio.h>
#include <stdlib.h>
#include "gen_matrix.h"
#include "my_malloc.h"
#include <cilk/cilk.h>

void result_from_row(double* result, int row, double* a, double* b, int dim_size){
  int k, x;
  for (int x = 0; x < dim_size; ++x) {
      double r = 0.0;
      for (int k = 0; k < dim_size; ++k) {
	      r += a[row * dim_size + k] *  b[k * dim_size + x];
      }
      result[row * dim_size + x] = r;
  }
  return;
}

// row major
void mm(double *result, double *a, double *b,  int dim_size) {
  int ctr = 0;
  for (int row = 0; row < dim_size; ++row) {
    // ctr++
    cilk_spawn result_from_row(result, row, a, b, dim_size);
    // if(ctr == __cilkrts_get_nworkers())
    //   cilk_sync;
    //   ctr = 0
  }
  cilk_sync;
}

// void result_from_cell(double* result, int row, int col, double* a, double* b, int dim_size){
//   int k, x;
//   double r = 0.0;
//   for (int k = 0; k < dim_size; ++k) {
//     r += a[row * dim_size + k] *  b[k * dim_size + col];
//   }
//   result[row * dim_size + col] = r;
//   return;
// }

// // row major
// void mm(double *result, double *a, double *b,  int dim_size) {
//   for (int row = 0; row < dim_size; ++row) {
//     for (int col = 0; col < dim_size; ++col) {
//       result_from_cell(result, row, col, a, b, dim_size);
//     }
//   }
  
// }

void print_matrix(double *result, int dim_size) {
  int x, y;
  for (y = 0; y < dim_size; ++y) {
    for (x = 0; x < dim_size; ++x) {
      printf("%f ", result[y * dim_size + x]);
    }
    printf("\n");
  }
  printf("\n");
}

int main(int argc, char *argv[]) {
  double **r;
  double **result;
  int i;
  int num_arg_matrices;

  //printf("clock %f: ", (double)start_time);

  if (argc != 4) {
    printf("usage: debug_perf test_set matrix_dimension_size\n");
    exit(1);
  }
  printf("workers here: %d\n", __cilkrts_get_nworkers());
  int debug_perf = atoi(argv[1]);
  int test_set = atoi(argv[2]);
  matrix_dimension_size = atoi(argv[3]);
  num_arg_matrices = init_gen_sub_matrix(test_set);
  
  // allocate arrays
  r = (double **)my_malloc(sizeof(double *) * num_arg_matrices);
  result = (double **)my_malloc(sizeof(double *) * 2);
  result[0] = (double *)my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size);
  result[1] = (double *)my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size);

  // get sub matrices
  for (i = 0; i < num_arg_matrices; ++i) {
    r[i] = (double *)my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size);
    if (gen_sub_matrix(0, test_set, i, r[i], 0, matrix_dimension_size - 1, 1, 0, matrix_dimension_size - 1, 1, 1) == NULL) {
      printf("inconsistency in gen_sub_matrix\n");
      exit(1);
    }
  }  

  // perform matrix multiplies
  int n = 0;

  mm(result[0], r[0], r[1], matrix_dimension_size);
  for (i = 2; i < num_arg_matrices; ++i) {
    mm(result[n ^ 0x1], result[n], r[i], matrix_dimension_size);
    n = n ^ 0x1;
  }
  cilk_sync;
  if (debug_perf == 0) {
    // print each of the sub matrices
    for (i = 0; i < num_arg_matrices; ++i) {
      printf("argument matrix %d\n", i);
      print_matrix(r[i], matrix_dimension_size);
    }
    printf("result matrix\n");
    print_matrix(result[n], matrix_dimension_size);
  } else {
    double sum = 0.0;

    for (i = 0; i < matrix_dimension_size * matrix_dimension_size; ++i) {
      sum += result[n][i];
    }
    printf("%f\n", sum);

    
  }

}

