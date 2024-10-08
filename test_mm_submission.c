// row major
void mm(double *result, double *a, double *b, int dim_size, int world_size, int world_rank, int debug_perf) {
  //TODO:    
  // double* recv_buffer = (double*) my_malloc(sizeof(double) * dim_size/world_size * dim_size);
    for(int total_iter = 0; total_iter < dim_size/world_size; total_iter ++){
      for (current_row = 0; current_row < dim_size/world_size; current_row++){
        for(current_col = 0; current_col < dim_size/world_size; current_col++){
          for(i = 0; i < dim_size; i ++){//for each value in the row
            result[current_row * dim_size + (current_col ) * dim_size/world_size + total_iter] += a[current_row * dim_size + i] * b[i + current_col * dim_size];
          }
        }
      }
      recv_loc = world_rank == 0 ? world_size - 1 : world_rank - 1;
      send_loc = (world_rank + 1) % world_size;
      if(total_iter != dim_size/world_size - 1 || debug_perf) {
        MPI_Sendrecv_replace(b, dim_size/world_size * dim_size, MPI_DOUBLE, send_loc,
                          recv_loc, 0, world_rank, 0,
                          MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    }
  }

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


  if (argc != 4) {
    printf("usage: debug_perf test_set matrix_dimension_size\n");
    exit(1);
  }

  MPI_Init(&argc, &argv);
  int world_size;
  int world_rank;
  int name_len;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  MPI_Comm_size(MPI_COMM_WORLD, &world_size); 
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); 
  MPI_Get_processor_name(processor_name, &name_len); 


  int debug_perf = atoi(argv[1]);
  int test_set = atoi(argv[2]);
  matrix_dimension_size = atoi(argv[3]);
  num_arg_matrices = init_gen_sub_matrix(test_set);
  
  // allocate arrays
  r = (double **)my_malloc(sizeof(double *) * num_arg_matrices);
  result = (double **)my_malloc(sizeof(double *) * 2);
  result[0] = (double *)my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size / world_size);
  result[1] = (double *)my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size / world_size);




  for (i = 0; i < num_arg_matrices; ++i) {
    r[i] = (double *)my_malloc(sizeof(double) * matrix_dimension_size * matrix_dimension_size/world_size);
    if(i == 0){
      if (gen_sub_matrix(world_rank, test_set, i, r[i], 0, matrix_dimension_size - 1, 1, world_rank, world_rank+matrix_dimension_size/world_size - 1, 1, 0) == NULL) {
      printf("inconsistency in gen_sub_matrix\n");
      exit(1);
    }
    else{
      if (gen_sub_matrix(world_rank, test_set, i, r[i], world_rank, world_rank + matrix_dimension_size/world_size - 1, 1, 0, matrix_dimension_size - 1, 1, 1) == NULL) {
        printf("inconsistency in gen_sub_matrix\n");
        exit(1);
      }
    }
  }  


  // perform matrix multiplies
  // void mm(double *result, double *a, double *b, int dim_size, int world_size, int world_rank, int debug_perf) {

  int n = 0;
   
  mm(result[0], r[0], r[1], matrix_dimension_size, world_size, world_rank, debug_perf);
  for (i = 2; i < num_arg_matrices; ++i) {
    mm(result[n ^ 0x1], result[n], r[i], matrix_dimension_size,  world_size, world_rank, debug_perf);
    n = n ^ 0x1;
  }
  double glo_sum = 0.0;
  double loc_sum = 0.0;
  for(int i = 0; i < dim_size/world_size * dim_size; i++){
    loc_sum += result[n][i];
  }

  MPI_Reduce(
        &loc_sum,
        &glo_sum,
        1,
        MPI_DOUBLE,
        MPI_SUM,
        0,
        MPI_COMM_WORLD);
  
  if (debug_perf == 0) {
    double* fin_result = (double*) malloc(sizeof(double) * dim_size * dim_size);
    MPI_Gather(
      result[n],
      dim_size*dim_size/world_size,
      MPI_DOUBLE,
      fin_result,
      dim_size*dim_size,
      MPI_DOUBLE,
      0,
      MPI_COMM_WORLD);
  }

  if(world_rank == 0) {
    if (debug_perf == 0) {
      
      // print each of the sub matrices
      for (i = 0; i < num_arg_matrices; ++i) {
        printf("argument matrix %d\n", i);
        print_matrix(r[i], matrix_dimension_size);
      }
      printf("result matrix\n");
      print_matrix(fin_result[n], matrix_dimension_size);
    } 
    else {
      double sum = glo_sum;
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
