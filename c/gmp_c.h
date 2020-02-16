#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "complex.h"
#include "complex_c.h"
#include <pthread.h>

#define NUM_THREADS 4 

volatile int busy;
volatile int stop;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;


pthread_t *calcc(char *buf, const char *Xstt, const char *Ystt, const char *Xend, const char *Yend, const char *It, const char *Wi, const char *He, const char *Bound);

void *func(void *args);
void *funcl(void *args);

pthread_t *calcc(char *buf, const char *Xstt, const char *Ystt, const char *Xend, const char *Yend, const char *It, const char *Wi, const char *He, const char *Bound){
  
  int rc
    , i;
  
  pthread_t *threads = malloc(sizeof(pthread_t) * NUM_THREADS);
  busy = 0;
  //malloc is importannt, because it seems like
  //the pointers will be freed before the receiving
  //thread starts parsing values
  char * xstt = malloc(strlen(Xstt) + 1);
  strcpy(xstt, Xstt);
  char * ystt = malloc(strlen(Ystt) + 1);
  strcpy(ystt, Ystt);
  char * xend = malloc(strlen(Xend) + 1);
  strcpy(xend, Xend);
  char * yend = malloc(strlen(Yend) + 1);
  strcpy(yend, Yend);
  char * it = malloc(strlen(It) + 1);
  strcpy(it, It);
  char * wi = malloc(strlen(Wi) + 1);
  strcpy(wi, Wi);
  char * he = malloc(strlen(He) + 1);
  strcpy(he, He);
  
  int h = strtol(he, NULL, 10);
  int w = strtol(wi, NULL, 10); 

  for( i = 0; i < NUM_THREADS; i++ ) {
      coords_t *crd = malloc(sizeof(coords_t));
      crd->Xstt = xstt;
      crd->Ystt = ystt;
      crd->Xend = xend;
      crd->Yend = yend;
      crd->Wi = wi;
      crd->He = he;
      crd->It = it;
      crd->buf = buf;
      crd->idx = i;
      
      void *f;
      if (strlen(xstt) < 20) 
        f = func;
      else
      {
        f = funcl;
      }

      rc = pthread_create(&threads[i], NULL, f, (void *)crd);
      if (rc) {
         printf("Error:unable to create thread, %d\n", rc);
         exit(-1);
      }
   }
  return threads;
}
