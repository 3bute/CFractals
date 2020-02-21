#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "complex.h"
#include "complexl.h"
#include <pthread.h>


volatile int busy;
volatile int stop;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void *func1(void *args);
void *funcl1(void *args);

void *func2(void *args);
void *funcl2(void *args);

pthread_t *calcc(char *buf, const char *Xstt, const char *Ystt, const char *Xend, const char *Yend, const char *It, const char *Wi, const char *He, const char *Bound, const char *J, const char *Jx, const char *Jy, const char *Pat){
  
  int rc, i;
  
  pthread_t *threads = malloc(sizeof(pthread_t) * NUM_THREADS);
  busy = 0;

  //malloc is importannt, because it seems like
  //the pointers will be freed before the receiving
  //thread starts parsing values
  char *xstt = malloc(strlen(Xstt) + 1)
     , *ystt = malloc(strlen(Ystt) + 1)
     , *xend = malloc(strlen(Xend) + 1)
     , *yend = malloc(strlen(Yend) + 1)
     , *it   = malloc(strlen(It) + 1)
     , *wi   = malloc(strlen(Wi) + 1)
     , *j    = malloc(strlen(J) + 1)
     , *he   = malloc(strlen(He) + 1)
     , *jx   = malloc(strlen(Jx) + 1)
     , *jy   = malloc(strlen(Jy) + 1);

  strcpy(xstt, Xstt);
  strcpy(ystt, Ystt);
  strcpy(xend, Xend);
  strcpy(yend, Yend);
  strcpy(it, It);
  strcpy(wi, Wi);
  strcpy(he, He);
  strcpy(j, J);
  strcpy(jx, Jx);
  strcpy(jy, Jy);
 
  float bound = strtof(Bound, NULL);
  int h = strtol(he, NULL, 10)
    , w = strtol(wi, NULL, 10) 
    , pat = strtol(Pat, NULL, 10); 

  for( i = 0; i < NUM_THREADS; i++ ) {

    coords_t *crd = malloc(sizeof(coords_t));
    crd->Xstt   = xstt;
    crd->Ystt   = ystt;
    crd->Xend   = xend;
    crd->Yend   = yend;
    crd->Wi     = wi;
    crd->He     = he;
    crd->It     = it;
    crd->J      = j;
    crd->Jx     = jx;
    crd->Jy     = jy;
    crd->bound  = bound;
    crd->buf    = buf;
    crd->idx    = i;
    
    void *f ;
    if (strlen(xstt) < 21) {
      if (pat) 
        f = func1;
      else 
        f = func2;
    } else {
      if (pat) 
        f = funcl1;
      else 
        f = funcl2;
    }
    rc = pthread_create(&threads[i], NULL, f, (void *)crd);
    if (rc) {
       printf("Error:unable to create thread, %d\n", rc);
       exit(-1);
    }
   }
  return threads;
}
