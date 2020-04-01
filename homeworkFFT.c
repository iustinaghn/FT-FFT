#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <complex.h>
#include <math.h>

typedef double complex cplx;
#define _USE_MATH_DEFINES

int P;
char* inFile;
char* outFile;
int N;
cplx* input;
cplx* output;

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
	//printf("%d\n", N);
	input = calloc(2 * N, sizeof(cplx));
	output = calloc(2 * N, sizeof(cplx));

	while((read = getline(&line, &len, fp)) != -1){
		input[++i] = atof(line);
	}	
	fclose(fp);
}

void _fft1(cplx* input, cplx* output, int step){
	if(step < N){
		_fft1(output, input, step * 2);
		_fft1(output + step, input + step, step * 2);	
			
		for(int i = 0; i < N; i += 2 * step){
			cplx angle = cexp(-I * M_PI * i / N) * output[i + step];
			input[i / 2] = output[i] + angle;
			input[(i + N)/2] = output[i] - angle;
		}
	}
}

void* par_fft1(void* args){
	(void) *(int*) args;
	switch(P){
		case 1: _fft1(input, output, 1);
				break;
		case 2:	_fft1(output, input, 2);
				break;
		case 4: _fft1(input, output, 4); 
				break;
	}
	return NULL;
}

void* par_fft2(void* args){
	(void) *(int*) args;
	switch(P){
		case 2:	_fft1(output + 1, input + 1, 2);
				break;
		case 4:_fft1(input + 2, output + 2, 4);
				break;
	}
	return NULL;	
}

void* par_fft3(void* args){
	(void) *(int*) args;
	_fft1(input + 1, output + 1, 4);
	
	return NULL;
}

void* par_fft4(void* args){
	(void) *(int*) args;
	_fft1(input + 3, output + 3, 4);

	return NULL;
}
void print(){
	FILE* fl;
	fl = fopen(outFile, "w");
	fprintf(fl, "%d\n", N);

	for(int i = 0; i < N; ++i)
		fprintf(fl, "%f %f\n", creal(input[i]), cimag(input[i]));

	fclose(fl);
}

int main(int argc, char * argv[]) {
	
	getArgs(argc, argv);
	init();

		for(int i = 0; i < N; ++i){
		output[i] = input[i];
	}
	pthread_t tid[P];
	int thread_id[P];
	for(int i = 0; i < P; ++i)
		thread_id[i] = i;
	for(int i = 0; i < P; ++i){
		if(i == 0)
			pthread_create(&(tid[i]), NULL, par_fft1, &(thread_id[i]));
		else if(i == 1)
			pthread_create(&(tid[i]), NULL, par_fft2, &(thread_id[i]));
		else if(i == 2)
			pthread_create(&(tid[i]), NULL, par_fft3, &(thread_id[i]));
		else
			pthread_create(&(tid[i]), NULL, par_fft4, &(thread_id[i]));
	}
	for(int i = 0; i < P; ++i)
		pthread_join(tid[i], NULL);

	if(P == 4){
		for(int i = 0; i < N; i += 4){
			cplx angle = cexp(-I * M_PI * i / N) * input[1 + i + 2];
			output[1 + i / 2] = input[i + 1] + angle;
			output[1 + (i + N)/2] = input[i + 1] - angle;
		}
		for(int i = 0; i < N; i += 4){
			cplx angle = cexp(-I * M_PI * i / N) * input[i + 2];
			output[i / 2] = input[i] + angle;
			output[(i + N)/2] = input[i] - angle;
		}
	}
	if(P == 2 || P == 4)
		for(int i = 0; i < N; i += 2){
			cplx angle = cexp(-I * M_PI * i / N) * output[i + 1];
			input[i / 2] = output[i] + angle;
			input[(i + N)/2] = output[i] - angle;
		}

	print();

	free(input);
	free(output);
	return 0;
}

