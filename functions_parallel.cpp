#include "functions_parallel.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstdlib>
#include <pthread.h>
int static N_Tests;
int read_matr(int n, double *mas, char **args) {
	FILE *f;
	int a, k;
	//printf("In read\n");
	k = atoi(args[3]);
	//printf ("filename = %s\n", args[4]);
	if (k == 0) {
		f = fopen(args[4], "r");
		if ( f == NULL){
			printf("Bad\n");
			fclose(f);
			return - 1;
		}
		printf ("FILE is OPENED\n");
		for (int i = 0; i < n*n; i ++){
			if (fscanf(f, "%lf", &mas[i]) != 1) return -1;
		}
		fclose(f);
		return 0;
	} else if (k < 5) {
		for (int i = 0; i < n; i ++) {
			for (int j = 0; j < n; j ++) {
				mas[i*n+j] = func(k, n, i+1, j+1);
			}
		}
		return 0;
	}
	return 2;
}
double func (int k, int n, int i, int j) {
	if (k == 1) {
		return n-fmax(i,j)+1;
	} else if (k == 2) {
		return fmax(i,j);
	} else if (k == 3) {
		return fabs(i-j);
	} else if (k == 4) {
		return 1/((double)i+(double)j-1);
	}
	return -1;
}
int find_nenul(int num_stolb, double* mas, int N, int num_str){
	double eps = 1e-27;
	for (int i = num_str; i < N; i ++){
		if (fabs(mas[i*N + num_stolb]) > eps){
			return i;
		}
	}
	return -1;
}
int write_matr (int n, int m, double *mas) {
	if (n < m) return -1;
	for (int i = 0; i < m; i ++){
		for (int j = 0; j < m; j ++){
			printf("%lf  ", mas[i * n + j]);
		}
		printf("\n");
	}
	return 0;
}
void make_nuls(int num_stolb, double* mas, int N, int num_str, int id_thread, double* mas_e){
	double elem, k;
	elem = mas[num_str * N + num_stolb];
	if (id_thread + 1 + num_str < N) {
		//printf ("id_thread in make_nuls  %d\n", id_thread);
		for (int i = 0; i < N; i ++){
			k = mas[(i * N_Tests + id_thread + 1 + num_str) * N + num_stolb];
			for (int j = 0; j < N; j ++){

				mas_e[(i * N_Tests + id_thread + 1 + num_str) * N + j] -= k * mas_e[num_str * N + j];
			}
			for (int j = num_str; j < N; j ++) {
				mas[(i * N_Tests + id_thread + 1 + num_str) * N + j] -= k * mas[num_str * N + j];
			}
			if ((i + 1) * N_Tests + id_thread + 1 + num_str > N - 1) break;
		}
	}
}
void* ThreadProc(void * data_new) {
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	static pthread_cond_t condvar_in = PTHREAD_COND_INITIALIZER;
	static pthread_cond_t condvar_out = PTHREAD_COND_INITIALIZER;
	static int Counter_in = 0, Counter_out = 0, Counter_in2 = 0, Counter_out2 = 0;
	DATA *data = (DATA *) data_new;
	int numLineBegin, numLineEnd, num_nen;
	int N = data->N;
	double eps = 1e-27, k, change;
	for (int i = 0; i < N; i ++) {
		//printf ("thread_id  %d\n", data->id_thread);
		//write_matr(3, 3, data->mas);
		//printf ("\n");
		pthread_mutex_lock(&mutex);
		Counter_in ++;

		if (Counter_in >= N_Tests) {
			Counter_out = 0;
			pthread_cond_broadcast(&condvar_in);
		} else {
			while (Counter_in < N_Tests) pthread_cond_wait(&condvar_in, &mutex);
		}
		Counter_out ++;
		if (Counter_out >= N_Tests) {
			Counter_in = 0;
			//printf ("In function 1\n");
			pthread_cond_broadcast(&condvar_out);
		} else {
			while (Counter_out < N_Tests) pthread_cond_wait(&condvar_out, &mutex);
		}
		pthread_mutex_unlock(&mutex);



		pthread_mutex_lock(&mutex);
		if (fabs(data->mas[i*N+i] - 1) > eps) {
			//printf("i %d = \n", i);
			num_nen = find_nenul(i, data->mas, N, i);

			if (num_nen != -1) {
				for (int j = i; j < N; j ++) {
					change = data->mas[i*N+j];
					data->mas[i*N+j] = data->mas[num_nen*N+j];
					data->mas[num_nen*N+j] = change;
					change = data->mas_e[i*N+j];
					data->mas_e[i*N+j] = data->mas_e[num_nen*N+j];
					data->mas_e[num_nen*N+j] = change;
				}

			} else {
				printf ("Mistake!\n");
			}
			if (fabs(data->mas[i * N + i]) > eps){
				k = 1 / data->mas[i * N + i];
				for (int j = 0; j < N; j ++){
					data->mas_e[i * N + j] *= k;
					data->mas[i * N + j] *= k;
				}
			} else {
				//printf("%lf\n", mas[i * N + i] );
			}

		}
		pthread_mutex_unlock(&mutex);
		//numLineBegin = ((N) * data->id_thread) / 4;
		//numLineEnd = ((N) * (data->id_thread + 1)) / 4 + 1;
		//printf("Make_Nuls\n");
		make_nuls(i, data->mas, N, i, data->id_thread, data->mas_e);
	}

	for (int i = N-1; i > -1; i--) {
		pthread_mutex_lock(&mutex);
		Counter_in2 ++;
		//printf ("COunter = %d\n", Counter_in2);
		if (Counter_in2 >= N_Tests) {
			Counter_out2 = 0;
			pthread_cond_broadcast(&condvar_in);
		} else {
			while (Counter_in2 < N_Tests) pthread_cond_wait(&condvar_in, &mutex);
		}
		Counter_out2 ++;
		if (Counter_out2 >= N_Tests) {
			Counter_in2 = 0;
			//printf ("In function 2\n");
			pthread_cond_broadcast(&condvar_out);
		} else {
			while (Counter_out2 < N_Tests) pthread_cond_wait(&condvar_out, &mutex);
		}
		pthread_mutex_unlock(&mutex);
		//printf("make_nuls_inv\n");
		make_nuls_inv(data->mas, N, i, data->id_thread, data->mas_e);
	}
	pthread_exit(NULL);
}
void make_nuls_inv(double* mas, int N, int num_str, int id_thread, double* mas_e) {
	double k;
	int num_stolb = num_str;
	//printf ("id_thread in make_nuls_inv  %d\n", id_thread);
	if (id_thread < num_str) {
		for (int i = 0; i < num_str; i ++){
			k = mas[( N_Tests * i + id_thread) * N + num_stolb];
			mas[( N_Tests * i + id_thread) * N + num_stolb] = 0;
			for (int j = 0; j < N; j ++){
				mas_e[(N_Tests * i + id_thread) * N + j] -= k * mas_e[num_str * N + j];
			}
			if ((N_Tests * (i + 1) + id_thread) > num_str - 1) break;
		}
	}
}

int solve(double *mas, int N, double *mas_e, int n_tests) {
	N_Tests = n_tests;
	pthread_t hThreads[N_Tests];
	int rc;
	for (int i = 0; i < N; i ++){
		for (int j = 0; j < N; j ++){
			if (i == j){
				mas_e[i * N + j] = 1;
			} else {
				mas_e[i * N + j] = 0;
			}
		}
	}

	DATA * data = (DATA*)malloc(N_Tests * sizeof(DATA));
	for(int i = 0; i < N_Tests; i++) {
		data[i].id_thread = i;
		data[i].mas = mas;
		data[i].mas_e = mas_e;
		data[i].N = N;
		rc = pthread_create(hThreads + i, 0, ThreadProc, data+i);
		if(rc) {
			printf("Mistake in creating threads!\n");
		}
	}



	for(int i = 0; i < N_Tests; i++) {
		if (pthread_join(hThreads[i], 0)) printf("Can't wait for process!\n");
	}
	free(data);
	return 0;
}

double calculate_mistake(int n, double *mas, double *ans) {
	double sum = 0;
	//double* mis = (double*)malloc(n*n*sizeof(double));
	for (int i = 0; i < n; i ++) {
		for (int j = 0; j < n; j ++) {
			double elem = 0;
			for (int k = 0; k < n; k ++) {
				elem = elem + mas[i*n + k]*ans[k*n+j];
				//printf("k = %d mas = %lf ans = %lf elem = %lf\n", k, mas[i*n + k], ans[k*n+j], elem);
			}
			//printf("1 elem = %lf\n", elem);
			if (i == j) {
				elem = elem - 1;
			}
			//printf("i = %d j = %d elem = %lf\n", i, j, elem);
			sum += elem*elem;
			//printf("SUM = %lf\n", sum);
		}
	}
	//write_matr(4, 4, mis);
	return sum;
}



































