#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <complex.h>
#include <math.h>

#define _USE_MATH_DEFINES

int P;
char* inFile;
char* outFile;
int N;
double* inputReal;
double* inputImag;
double* outputReal;
double* outputImag;


void getArgs(int argc, char **argv){
	if(argc < 4) {
		printf("Not enough paramters: ./program input-file output-file P\n");
		exit(1);
	}
	inFile = argv[1];
	outFile = argv[2];
	P = atoi(argv[3]);
}

void init(){
	FILE* fp;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	int i = -1;
	fp = fopen(inFile, "r");
	if(fp == NULL)
		exit(1);
	
	read = (getline(&line, &len, fp));
	N = atoi(line);	
	inputReal = malloc(sizeof(double) * N);
	inputImag = malloc(sizeof(double) * N);
	for(int i = 0; i < N; ++i)
		inputImag[i]  = 0;

	outputReal = malloc(sizeof(double) * N);
	outputImag = malloc(sizeof(double) * N);
	while((read = getline(&line, &len, fp)) != -1){
		inputReal[++i] = atof(line);
	}	
	fclose(fp);
}

void* par_dft(void *args){
	int tid = *(int*) args;
	int start   = tid * ceil((double)N / P);
	int end     = fmin(N, (tid + 1) * ceil((double)N / P));

	FILE* fl;
	fl = fopen(outFile, "w");
	fprintf(fl, "%d\n", N);

	for( long long i = start; i < end; ++i){
		double sumReal = 0.0;
		double sumImag = 0.0;
		for(int j = 0; j < N; ++j){
			double angle = 2 * M_PI * j * i / N;
			sumReal += inputReal[j] * cos(angle) + inputImag[j] * sin(angle);
			sumImag += -inputReal[j] * sin(angle) + inputImag[j] * cos(angle);

		}
		outputReal[i] = sumReal;
		outputImag[i] = sumImag;
	}
	for(int i = 0; i < N; ++i)
		fprintf(fl, "%f %f\n", outputReal[i], outputImag[i]);
	fclose(fl);	
	
}
int main(int argc, char * argv[]) {
	
	getArgs(argc, argv);
	init();

	pthread_t tid[P];
	int thread_id[P];

	for(int i = 0; i < P; ++i)
		thread_id[i] = i;

	for(int i = 0; i < P; ++i)
		pthread_create(&(tid[i]), NULL, par_dft, &(thread_id[i]));
	for(int i = 0; i < P; ++i)
		pthread_join(tid[i], NULL);
	return 0;
}
