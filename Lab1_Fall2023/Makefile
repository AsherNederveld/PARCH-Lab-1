test_mm: test_mm.c gen_matrix.c my_malloc.c gen_matrix.h my_malloc.h
	gcc -g -DDEBUG test_mm.c gen_matrix.c my_malloc.c -o test_mm

enter_idev:
	idev -N 2 -n 128

run_debug:
	ibrun -n 2 ./test_mm 0 0 2048
	
run_performance:
	./test_mm 1 0 16
	./test_mm 1 1 16
	./test_mm 1 2 16
	./test_mm 1 3 16
	./test_mm 1 4 16
	./test_mm 1 5 16
	./test_mm 1 6 16

run_performance_one_core:
	ibrun -n 1 ./test_mm 1 0 2048

clean:
	rm *~; rm *.exe

