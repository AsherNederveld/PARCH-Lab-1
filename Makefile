test_mm: test_mm.c gen_matrix.c my_malloc.c gen_matrix.h my_malloc.h
	mpicc -O3 -fno-tree-vectorize -mno-avx -mno-avx2 -mno-fma -mno-sse2 -mno-sse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2 test_mm.c gen_matrix.c my_malloc.c -o test_mm
	gcc -g -DDEBUG test_mm.c gen_matrix.c my_malloc.c -o test_mm

enter_idev:
	idev -N 1 -n 4

run_debug:
	ibrun -n 2 ./test_mm 0 0 2048
	
run_performance:
	ibrun -n 4 ./test_mm 1 0 16
	ibrun -n 4 ./test_mm 1 1 16
	ibrun -n 4 ./test_mm 1 2 16
	ibrun -n 4 ./test_mm 1 3 16
	ibrun -n 4 ./test_mm 1 4 16
	ibrun -n 4 ./test_mm 1 5 16
	ibrun -n 4 ./test_mm 1 6 16S

run_performance_one_core:
	ibrun -n 1 ./test_mm 1 0 2048

clean:
	rm *~; rm *.exe

