test_mm: test_mm.c gen_matrix.c my_malloc.c gen_matrix.h my_malloc.h
	mpicc -O3 -fno-tree-vectorize -mno-avx -mno-avx2 -mno-fma -mno-sse2 -mno-sse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2 test_mm.c gen_matrix.c my_malloc.c -o test_mm
	# gcc -g -DDEBUG test_mm.c gen_matrix.c my_malloc.c -o test_mm

enter_idev:
	idev -N 2 -n 100

run_debug:
	ibrun -n 4 ./test_mm 0 0 8
	

run_performance:
	ibrun -n 256 ./test_mm 1 0 8

clean:
	rm *~; rm *.exe

