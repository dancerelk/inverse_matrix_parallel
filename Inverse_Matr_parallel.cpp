#include <stdio.h>
#include <stdlib.h>
#include "functions_parallel.h"
#include <time.h>
#include <sys/time.h>

int main (int argc, char *argv[]) {
	struct timeval time_begin, time_end;
	double *mas, *ans, *mas_new, mistake;
	int n, m, k, n_tests;
	char *filename;
	clock_t end, begin;
	//printf("%d\n", argc);
	if ((argc > 6) or (argc < 5)) {
		printf("Mistake\n");
		return 1;
	}
	
	n = atoi(argv[1]);
	m = atoi(argv[2]);
	mas = (double*)malloc(n*n*sizeof(double));
	mas_new = (double*)malloc(n*n*sizeof(double));
	if (mas == NULL) return -1;
	if (mas_new == NULL) return -1;
	//printf("In main2\n");
	int res = read_matr(n, mas, argv);
	if (res == 2) {
		printf("No such formula\n");
		return 1;
	}
	if (res == -1) {
		printf("Not enough!\n");
		return 1;
	}
	for (int i = 0; i < n*n; i ++) {
		mas_new[i] = mas[i]; 
	}
	write_matr(n, m, mas);
	ans = (double*)malloc(n*n*sizeof(double));
	if (ans == NULL) return -1;
	gettimeofday(&time_begin, NULL);
	n_tests = atoi(argv[argc - 1]);
	if (solve(mas, n, ans, n_tests) == -1) {
		printf("Mistake!\n");
		return -1;
	}
	gettimeofday(&time_end, NULL);
	printf("\n");
	printf("Microseconds: %ld\n", time_end.tv_usec - time_begin.tv_usec);
	printf("\n");
	write_matr(n, m, ans);
	mistake = calculate_mistake(n, mas_new, ans);
	printf("\n");
	printf("Mistake = %e\n", mistake);
	free(ans);
	free(mas);
	free(mas_new);
	return 0;
}


	