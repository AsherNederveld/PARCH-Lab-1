#include <stdio.h>
#include <stdlib.h>
#include "gen_matrix.h"
#include "my_malloc.h"
#include <mpi.h>
#include <unistd.h> // For sleep



void print2D(double* a, int dim_size, int world_size){
  // printf("Begin Dump: \n");
  for(int i = 0; i < dim_size * dim_size / world_size; i ++){
    printf("%f, ", a[i]);
  }
  printf("\n");
}

// row major
void mm(double *result, double *a, double *b, int dim_size, int world_size, int world_rank, int debug_perf) {
    for(int i = 0; i < (dim_size/world_size) * dim_size; i++) {
        result[i] = 0.0;
    }
    for(int total_iter = 0; total_iter < world_size; total_iter ++){
      for (int current_row = 0; current_row < dim_size/world_size; current_row++){
        for(int current_col = 0; current_col < dim_size/world_size; current_col++){
          for(int i = 0; i < dim_size; i ++){//for each value in the row
            int result_index = (current_row * dim_size + (current_col ) + (((total_iter+world_rank) % world_size) * dim_size /world_size));
            if(i == 0){
              result[result_index] = a[current_row * dim_size + i] * b[i + current_col * dim_size];
            }
            else result[result_index] += a[current_row * dim_size + i] * b[i + current_col * dim_size];
          }
          /*
          int col_idx = current_col + (total_iter * dim_size/world_size);
          int result_idx = current_row * dim_size + col_idx;
          
          if(i == 0) {
              result[result_idx] = a[current_row * dim_size + i] * b[i + current_col * dim_size];
          }
          else {
              result[result_idx] += a[current_row * dim_size + i] * b[i + current_col * dim_size];
          }
                  }*/
        }
      }
      int recv_loc = world_rank == 0 ? world_size - 1 : world_rank - 1;
      int send_loc = world_rank == world_size -1 ? 0 : (world_rank + 1);
      if(total_iter != world_size - 1 || !debug_perf) {
        MPI_Sendrecv_replace(b, dim_size/world_size * dim_size, MPI_DOUBLE,
                          recv_loc, 0, send_loc, 0,
                          MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    }
  }

void print_matrix(double *result, int dim_size) {
  int x, y;
  for (y = 0; y < dim_size; ++y) {
    for (x = 0; x < dim_size; ++x) {
      printf("%f ", result[x + y* dim_size ]);
    }
    printf("\n");
  }
  printf("\n");
}

void print_matrix_frag(double *result, int dim_size) {
  int x, y;
  for (y = 0; y < dim_size; ++y) {
    for (x = 0; x < dim_size; ++x) {
      printf("%f ", result[x * dim_size + y]);
    }
    printf("\n");
  }
  printf("\n");
}

void printColMajor(double *result, int dim_size) {
  int x,y;
  for(y = 0; y < dim_size; y++){
    for(x = 0; x < dim_size; x++){
        printf("%f ", result[x + y * dim_size ]);
    }
    printf("\n");
  }
  printf("\n");
}

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);
  int world_size;
  int world_rank;
  int name_len;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Comm_size(MPI_COMM_WORLD, &world_size); 
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); 
  MPI_Get_processor_name(processor_name, &name_len); 
  double ** arg_arrs;
  double **r;
  double **result;
  int i;
  int num_arg_matrices;
  double* fin_result;

  if (argc != 4) {
    printf("usage: debug_perf test_set matrix_dimension_size\n");
    exit(1);
  }
  printf("Hello from processor %s, rank %d out of %d processors\n",
            processor_name, world_rank, world_size);
  fflush(stdout);
  int debug_perf = atoi(argv[1]);
  int test_set = atoi(argv[2]);
  matrix_dimension_size = atoi(argv[3]);
  num_arg_matrices =  init_gen_sub_matrix(test_set);
  
  // // allocate arrays
  //   double *gen_sub_matrix(int pid,        // the process id of the job calling.  This should be a number between 0 and num processes - 1
	// 	       int test_set,   // the test set command line argument
	// 	       int matrix_num, // the matrix number [0, num_arg_matrices - 1] 
	// 	       double *result, // a pointer to storage for the flattened array of the requested sub matrix
	// 	       int x_lo,       // the starting x value of the requested sub matrix
	// 	       int x_hi,       // the ending x value of the requested sub matrix
	// 	       int x_stride,   // the stride in the x dimension
	// 	       int y_lo,       // the starting y value of the requested sub matrix
	// 	       int y_hi,       // the ending y value of the requested sub matrix
	// 	       int y_stride,   // the stride in the y dimension
	// 	       int row_major_p)// whether the returned sub matrix is in row_major or column_major order
  // for (i = 0; i < num_arg_matrices; ++i) {

  r = (double **)my_malloc(sizeof(double *) * num_arg_matrices);
  result = (double **)my_malloc(sizeof(double *) * 2);
  result[0] = (double *)my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size / world_size);
  result[1] = (double *)my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size / world_size);
  for (int i = 0; i < num_arg_matrices; ++i) {
    r[i] = (double *)my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size/world_size);
    if(i != 0){
        if (gen_sub_matrix(world_rank, test_set, i, r[i], 0, matrix_dimension_size - 1, 1, world_rank * matrix_dimension_size/world_size , world_rank * matrix_dimension_size/world_size+matrix_dimension_size/world_size - 1, 1, 1) == NULL) {
        printf("inconsistency in gen_sub_matrix\n");
        exit(1);
      }
    }
    else{
      if (gen_sub_matrix(world_rank, test_set, i, r[i], world_rank* matrix_dimension_size/world_size , world_rank* matrix_dimension_size/world_size + matrix_dimension_size/world_size - 1, 1, 0, matrix_dimension_size - 1, 1, 0) == NULL) {
        printf("inconsistency in gen_sub_matrix\n");
        exit(1);
      }
    }
  }  

  int n = 0;
   
  mm(result[0], r[0], r[1], matrix_dimension_size, world_size, world_rank, debug_perf);
//     if(world_rank == 1){
//  sleep(1) ;   }
//     if(world_rank == 2){
// sleep(2);    }
//     if(world_rank == 3){
// sleep(3) ;   }
    
//     for(int j =0; j < matrix_dimension_size * matrix_dimension_size / world_size; j++){
//       if(j%16 == 0){
//         printf("\n");
//       }
//       printf("%f ",result[0][j]);
//     }
//     printf("\n");
    
  for (int i = 2; i < num_arg_matrices; ++i) {
    mm(result[n ^ 0x1], result[n], r[i], matrix_dimension_size,  world_size, world_rank, debug_perf);
           

    // if(world_rank == 0){
    //   printf("\n Begin \n");
    //   sleep(4);
    // }
    // if(world_rank == 1){
    //     sleep(5) ;   
    //     }
    // if(world_rank == 2){
    //   sleep(6);    
    //   }
    // if(world_rank == 3){
    //   sleep(7) ;   
    //   }
    // for(int j =0; j < matrix_dimension_size * matrix_dimension_size / world_size; j++){
    //   if(j%16 == 0){
    //     printf("\n");
    //   }
    //   printf("%f ",result[n ^ 0x1][j]);
    // }
    // printf("\n");

    n = n ^ 0x1;
  }
  double* glo_sum = (double*)my_malloc(sizeof(double));
  double* loc_sum = (double*)my_malloc(sizeof(double));
  *glo_sum = 0.0;
  *loc_sum = 0.0;
  for(int i = 0; i < matrix_dimension_size/world_size * matrix_dimension_size; i++){
    *loc_sum += result[n][i];
  }

  MPI_Reduce(
        loc_sum,
        glo_sum,
        1,
        MPI_DOUBLE,
        MPI_SUM,
        0,
        MPI_COMM_WORLD);
  
  if (debug_perf == 0) {
    // if (world_rank == 0){
      fin_result = (double*) my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size);
      arg_arrs = (double**) my_malloc(sizeof(double*)*num_arg_matrices);
      for(int i = 0; i < num_arg_matrices; i ++){
        arg_arrs[i] = (double*) my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size);
      }
    // }
    for(int i = 0; i < num_arg_matrices; i++){
      MPI_Gather(
      r[i],
      matrix_dimension_size*matrix_dimension_size/world_size,
      MPI_DOUBLE,
      arg_arrs[i],
      matrix_dimension_size*matrix_dimension_size/world_size,
      MPI_DOUBLE,
      0,
      MPI_COMM_WORLD);
    }
    MPI_Gather(
      result[1],
      matrix_dimension_size*matrix_dimension_size/world_size,
      MPI_DOUBLE,
      fin_result,
      matrix_dimension_size*matrix_dimension_size/world_size,
      MPI_DOUBLE,
      0,
      MPI_COMM_WORLD);

      
  }

  if(world_rank == 0) {
    if (debug_perf == 0) {
      
      //print each of the sub matrices

      for (int i = 0; i < num_arg_matrices; ++i) {
        if(i == 0){
          printf("argument matrix %d\n", i);
          print_matrix(arg_arrs[i], matrix_dimension_size);
        }
        else{
          printf("argument matrix %d\n", i);
          printColMajor(arg_arrs[i], matrix_dimension_size);
        }
      }
      printf("result matrix\n");
      print_matrix(fin_result, matrix_dimension_size);
    } 
    else {
      double sum = *glo_sum;
      printf("%f\n", sum);
    }
}

MPI_Finalize();
}






  //TODO:

  /*
  double *gen_sub_matrix(int pid,        // the process id of the job calling.  This should be a number between 0 and num processes - 1
		       int test_set,   // the test set command line argument
		       int matrix_num, // the matrix number [0, num_arg_matrices - 1] 
		       double *result, // a pointer to storage for the flattened array of the requested sub matrix
		       int x_lo,       // the starting x value of the requested sub matrix
		       int x_hi,       // the ending x value of the requested sub matrix
		       int x_stride,   // the stride in the x dimension
		       int y_lo,       // the starting y value of the requested sub matrix
		       int y_hi,       // the ending y value of the requested sub matrix
		       int y_stride,   // the stride in the y dimension
		       int row_major_p)// whether the returned sub matrix is in row_major or column_major order
  for (i = 0; i < num_arg_matrices; ++i) {
    r[i] = (double *)my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size);
    if (gen_sub_matrix(0, test_set, i, r[i], 0, matrix_dimension_size - 1, 1, 0, matrix_dimension_size - 1, 1, 1) == NULL) {
      printf("inconsistency in gen_sub_matrix\n");
      exit(1);
    }
  }  

{
  */
  // get sub matrices
