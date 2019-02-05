#include <stdlib.h>
#include <pthread.h>
#include <thread>
#include <chrono>

int* eatStatus;
pthread_mutex_t esLock;
pthread_mutex_t midLock;
bool priorityIsWaiting;
pthread_mutex_t piwLock;

struct input {
  pthread_mutex_t* l;
  pthread_mutex_t* r;
  int id;
};

struct printin {
  int nPhilos;
};

void* pworker(void* arg) {
  struct printin in = *((printin*)arg);
  int nPhilos = in.nPhilos;
  int copyes[in.nPhilos];

  while(1) {
    pthread_mutex_lock(&esLock);
    for (int j = 0; j < nPhilos; j++) {
      copyes[j] = eatStatus[j];
    } 
    pthread_mutex_unlock(&esLock);
    for (int j = 0; j < nPhilos; j++) {
      printf("philo %d: %d, ", j, copyes[j]);
    }
    printf("\n\n");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return NULL;
}

void* worker(void* arg) {
  pthread_mutex_t* first;
  pthread_mutex_t* second;

  struct input in = *((struct input*)arg);
  if (true) {
    first = in.r;
    second = in.l;
  } else {
    first = in.l;
    second = in.r;
  }

  while (1) {
      pthread_mutex_lock(first);
      pthread_mutex_lock(second);
      pthread_mutex_lock(&esLock);
      eatStatus[in.id] = true;
      pthread_mutex_unlock(&esLock);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      pthread_mutex_lock(&esLock);
      eatStatus[in.id] = false;
      pthread_mutex_unlock(&esLock);
      pthread_mutex_unlock(first);
      pthread_mutex_unlock(second);
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return NULL;
}

void* forkInMiddle(void* arg) {
  pthread_mutex_t* first;
  pthread_mutex_t* second;

  struct input in = *((struct input*)arg);
  first = in.r;
  second = in.l;

  while (1) {
      first = in.r;
      second = in.l;
      if (pthread_mutex_trylock(&midLock) == 0) {
        first = &midLock;
        pthread_mutex_lock(second);; 
      } else {
        pthread_mutex_lock(first);
        pthread_mutex_lock(second);
      }
      pthread_mutex_lock(&esLock);
      eatStatus[in.id] = true;
      pthread_mutex_unlock(&esLock);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      pthread_mutex_lock(&esLock);
      eatStatus[in.id] = false;
      pthread_mutex_unlock(&esLock);
      pthread_mutex_unlock(first);
      pthread_mutex_unlock(second);
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return NULL;
}

void* givePriority(void* arg) {
  pthread_mutex_t* first;
  pthread_mutex_t* second;
  bool shouldIWait = true;

  struct input in = *((struct input*)arg);
  if (true) {
    first = in.r;
    second = in.l;
  } else {
    first = in.l;
    second = in.r;
  }

  while (1) {
      while(shouldIWait) {
        pthread_mutex_lock(&piwLock);
        shouldIWait = priorityIsWaiting;
        pthread_mutex_unlock(&piwLock);
      }
      pthread_mutex_lock(first);
      while(shouldIWait) {
        pthread_mutex_lock(&piwLock);
        shouldIWait = priorityIsWaiting;
        pthread_mutex_unlock(&piwLock);
      }
      pthread_mutex_lock(second);
      pthread_mutex_lock(&esLock);
      eatStatus[in.id] = true;
      pthread_mutex_unlock(&esLock);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      pthread_mutex_lock(&esLock);
      eatStatus[in.id] = false;
      pthread_mutex_unlock(&esLock);
      pthread_mutex_unlock(first);
      pthread_mutex_unlock(second);
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return NULL;
}

void* hasPriority(void* arg) {
  pthread_mutex_t* first;
  pthread_mutex_t* second;

  struct input in = *((struct input*)arg);
  if (true) {
    first = in.r;
    second = in.l;
  } else {
    first = in.l;
    second = in.r;
  }

  while (1) {
      pthread_mutex_lock(&piwLock);
      priorityIsWaiting = true;
      pthread_mutex_unlock(&piwLock);
      pthread_mutex_lock(first);
      pthread_mutex_lock(second);
      pthread_mutex_lock(&esLock);
      eatStatus[in.id] = true;
      pthread_mutex_unlock(&esLock);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      pthread_mutex_lock(&esLock);
      eatStatus[in.id] = false;
      pthread_mutex_unlock(&esLock);
      pthread_mutex_unlock(first);
      pthread_mutex_unlock(second);
      pthread_mutex_lock(&piwLock);
      priorityIsWaiting = false;
      pthread_mutex_unlock(&piwLock);
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return NULL;
}


int main(int argc, char** argv) {
  int nPhilo = atoi(argv[1]);
  int arg = atoi(argv[2]);
  struct input in[nPhilo];
  pthread_mutex_t lock[nPhilo];
  pthread_t threads[nPhilo + 1];
  eatStatus = (int*)malloc(nPhilo * sizeof(int));
  pthread_mutex_init(&esLock, NULL);
  void* (*whichThread[nPhilo])(void*);
  for (int j = 0; j < nPhilo; j++) {
    whichThread[j] = &worker;
  }

  if (arg == 1) {
    priorityIsWaiting = true;
    pthread_mutex_init(&piwLock, NULL);
    whichThread[0] = &hasPriority;
    for (int j = 1; j < nPhilo; j++) {
      whichThread[j] = givePriority;
    }
  } else if (arg == 2) {
    pthread_mutex_init(&midLock, NULL);
    for (int j = 1; j < nPhilo; j++) {
      whichThread[j] = forkInMiddle;
    }
  }

  for (int j = 0; j < nPhilo; j++) {
    pthread_mutex_init(&lock[j], NULL);
  }
  for (int j = 0; j < nPhilo; j++) {
    in[j] = {&lock[j], &lock[(j + 1) % nPhilo], j};
    pthread_create(&threads[j], NULL, whichThread[j], &in[j]);
  }
  struct printin pin = {nPhilo};
  pthread_create(threads, NULL, &pworker, &pin);
  while(1);
}
