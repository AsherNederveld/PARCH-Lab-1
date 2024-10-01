test_mm: test_mm.c gen_matrix.c my_malloc.c gen_matrix.h my_malloc.h
	/work/08382/mengtian/ls6/cilk/bin/clang -fopencilk -O3 -fno-tree-vectorize -mno-avx -mno-avx2 -mno-mmx -mno-fma -mno-sse3 -mno-sse4 -mno-sse4.1 -mno-sse4.2 test_mm.c my_malloc.c gen_matrix.c -o test_mm
	# gcc -g -DDEBUG test_mm.c gen_matrix.c my_malloc.c -o test_mm

enter_idev:
	idev -N 2 -n 128

run_debug:
	export CILK_NWORKERS=8
	./test_mm 0 0 8

run_performance:
	CILK_NWORKERS=128 time ./test_mm 1 0 2048

clean:
	rm *~; rm *.exe

