#include <iostream>
#include <stdlib.h>
#include <pthread.h>

struct input {
  int start;
  int end;
  int p;
  bool* isPrime;
};

void* worker(void* arg) {
  input in = *((input*)arg);

  for (int p = in.start; p <= in.end; p+=in.p) {
    in.isPrime[p] = false;
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  int nthreads = atoi(argv[1]);
  int max = atoi(argv[2]);
  bool* isPrime = (bool*)malloc(max * sizeof(bool));

  int indices[nthreads + 1];
  pthread_t threads[nthreads];
  struct input in[nthreads];

  isPrime[0] = false;
  isPrime[1] = false;
  for (int i = 2; i <= max; i++) {
      isPrime[i] = true;
  }

  for (int i = 2; i * i <= max; i++) {
    if (isPrime[i]) {
      int workerLen = ((max / i) - i) / nthreads;
      for (int j = 0; j < nthreads; j++) {
        indices[j] = j * i * workerLen  + i * i;
      }
      indices[nthreads] = max;
      for (int j = 0; j < nthreads; j++) {
        // there is redundancy here since index[j + 1] checked twice
        // need to do so to prevent last value from missing though
        in[j] = {indices[j], indices[j + 1], i, isPrime};
        pthread_create(&threads[j], NULL, &worker, &in[j]);
      }
      for (int j = 0; j < nthreads; j++) {
        pthread_join(threads[j], NULL);
      }
    }
  }
/*
  std::cout << std::endl;
  for (int i = 0; i <= max; i++) {
    if (isPrime[i]) {
      std::cout << i << " ";
    }
  }
  std::cout << std::endl;
*/
}
