#pragma once

int read_matr(int n, double *mas, char **args);
double func (int k, int n, int i, int j); 
int write_matr (int n, int m, double *mas);
int solve(double *mas, int n, double *ans, int n_tests);
int find_nenul(int num_stolb, double* mas, int N, int num_str);
void make_nuls(int num_stolb, double* mas, int N, int num_str, int id_thread, double* mas_e);
double calculate_mistake(int n, double *mas, double *ans);
void make_nuls_inv(double* mas, int N, int num_str, int id_thread, double* mas_e);
void* ThreadProc(void * data_new);
struct DATA {
	int id_thread;
	double *mas;
	double *mas_e;
	int N;
};