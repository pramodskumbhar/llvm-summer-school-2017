#define N (1024 * 1024 * 1024)

typedef int v4fl __attribute__ ((vector_size (16)));

void vector_addition(v4fl *A, v4fl *B) {
	for (long i = 0; i < N/8; i++)
    	A[i] += B[i];
}
