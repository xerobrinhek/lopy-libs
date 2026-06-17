#include <pthread.h>
#include <stdlib.h>

typedef struct {
    void* obj;
    void* (*func)(void*);
} thread_args_t;

void* thread_start(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    args->func(args->obj);
    free(args);
    return NULL;
}

long long thread_create(void* obj, void* (*func)(void*)) {
    pthread_t tid;
    thread_args_t* args = malloc(sizeof(thread_args_t));
    args->obj = obj;
    args->func = func;
    pthread_create(&tid, NULL, thread_start, args);
    return (long long)tid;
}

void thread_join(long long handle) {
    pthread_join((pthread_t)handle, NULL);
}