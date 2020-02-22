#include <math.h>
#include <time.h>
#include <pthread.h>
#include "coords.h"

#define NUM_THREADS 4

extern volatile int busy;
extern volatile int stop;
extern pthread_mutex_t mutex1;

typedef struct {
  long double Re;
  long double Im;
} Complex_t;

int cc(long double re,
           long double im,
           Complex_t* z)
{
  z->Re = re;
  z->Im = im;
  return 0;
}

int squarec(Complex_t* z)
{
  long double temp = z->Im;
  z->Im = z->Re * z->Im + z->Im * z->Re;
  z->Re = z->Re * z->Re - temp * temp;
  return 0;
}

long double getR(Complex_t* z)
{
  return sqrtl(z->Re * z->Re + z->Im * z->Im);
}

int addc(Complex_t* a, Complex_t* b)
{
  a->Re = a->Re + b->Re; 
  a->Im = a->Im + b->Im;
  return 0;
}

long double map(long double a, long double b, long double c, long double d, long double e){
  return (a - b) / (c - b) * (e - d) + d;  
}

void *compute(int x, 
              int y, 
              int wi, 
              int he, 
              long double xstt, 
              long double xend, 
              long double ystt, 
              long double yend, 
              long it, 
              int julia, 
              long double jx, 
              long double jy, 
              float bound, 
              char *buf) {
  if (stop) {
    busy--;
    pthread_exit(0);
    return 0;
  }
  
  long double i, j;

  Complex_t *z = malloc(sizeof(Complex_t));
  Complex_t *c = malloc(sizeof(Complex_t));

  i = map(x, 0.0, wi, xstt, xend);
  j = map(y, 0.0, he, ystt, yend);
  
  if (!julia) {
    cc(0.0, 0.0, z);
    cc(  i,   j, c);
  } else {
    cc(jx, jy, c);
    cc( i,  j, z);
  }
  
  long k, delta;
  int added = 0;

  for (k = 0; k < it; ++k){
    squarec(z);
    addc(z, c);
    if (getR(z) > bound){
      added = 1;
      delta = k * 100 / it;        
      pthread_mutex_lock( &mutex1 );
      sprintf(buf + strlen(buf),
          "{\"x\":%i,\"y\":%i,\"d\":%ld};", x, y, delta);
      pthread_mutex_unlock( &mutex1 );
      break ;
    }
  }
  if (!added){ 
      pthread_mutex_lock( &mutex1 );
      sprintf(buf + strlen(buf),
          "{\"x\":%i,\"y\":%i,\"d\":0};", x, y);
      pthread_mutex_unlock( &mutex1 );
  }
  free(z);
  free(c);
}

void *func(void *args) {
  //in order to stop thread the cancel type should
  //be provided as async or deferred
  int prevType;
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &prevType);
  
  busy++;
  coords_t *crd = (coords_t *) args;

  long it = strtol(crd->It, NULL, 10);
  int  he = strtol(crd->He, NULL, 10)
     , wi = strtol(crd->Wi, NULL, 10) 
     , julia = 0;
  
  long double xstt = strtold(crd->Xstt, NULL)
            , ystt = strtold(crd->Ystt, NULL)
            , xend = strtold(crd->Xend, NULL)
            , yend = strtold(crd->Yend, NULL)
            , jx = 0
            , jy = 0;

  if (!strcmp(crd->J, "true")) {
    julia = 1;
    jx = strtold(crd->Jx,  NULL);
    jy = strtold(crd->Jy, NULL);
  } else {
    julia = 0;
  }
    
  int y = 0
    , x 
    , xa; 

  switch (crd->idx) {
    case 0:
      x   = wi / 2;
      xa  = wi / 2 + wi / NUM_THREADS; 
      for (x; x < xa ; x++) {
        for (y = 0; y < he; y++) {
          compute(x, y, wi, he, xstt, xend, ystt, yend, it, julia, jx, jy, crd->bound, crd->buf);      
        }
      }
      break;

    case 1:
      x  = wi / 2 + wi / NUM_THREADS; 
      xa  = wi; 
      for (x; x < xa ; x++) {
        for (y = 0; y < he; y++) {
          compute(x, y, wi, he, xstt, xend, ystt, yend, it, julia, jx, jy, crd->bound, crd->buf);      
        }
      }
      break;
      
    case 2:
      x   = wi / 2;
      xa  = wi / 2 - wi / NUM_THREADS; 
      for (x; x > xa ; x--) {
        for (y = 0; y < he; y++) {
          compute(x, y, wi, he, xstt, xend, ystt, yend, it, julia, jx, jy, crd->bound, crd->buf);      
        }
      }
      break;

    case 3:
      x  = wi / 2 - wi / NUM_THREADS; 
      xa  = 0; 
      for (x; x > xa ; x--) {
        for (y = 0; y < he; y++) {
          compute(x, y, wi, he, xstt, xend, ystt, yend, it, julia, jx, jy, crd->bound, crd->buf);      
        }
      }
      break;
  }
  free(crd);
  busy--;
  pthread_mutex_lock( &mutex1 );
  sprintf(crd->buf + strlen(crd->buf), "f");
  pthread_mutex_unlock( &mutex1 );
  return NULL;
}
