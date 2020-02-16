#include <math.h>
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

void *func(void *args) {
  int prevType;
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &prevType);
  
  coords_t *crd = (coords_t *) args;
  
  //in order to stop thread the cancel type should
  //be provided as async or deferred
  busy++;
  printf("%s\n", "thread has started!"); 
  long it = strtol(crd->It, NULL, 10);
  int he = strtol(crd->He, NULL, 10);
  int wi = strtol(crd->Wi, NULL, 10); 
  
  long double xstt = strtold(crd->Xstt, NULL);
  long double ystt = strtold(crd->Ystt, NULL);
  long double xend = strtold(crd->Xend, NULL);
  long double yend = strtold(crd->Yend, NULL);
  
  int shortest = ( wi > he ) ? he : wi
    , half = shortest / 2;

  //allocate start & end point on diagonal
  int n   = half - (crd->idx * half / NUM_THREADS )
    , end = half - ((crd->idx + 1) * half / NUM_THREADS );

  for (n; n > end; --n) {

    int  x     = n
      , _x     = n
      ,  y     = n
      , _y     = n
      , done   = 0
      , top    = 0
      , right  = 0
      , bottom = 0
      , left   = 0;

    while (!done) {
      if (stop) {
        busy--;
        pthread_exit(0);
        return 0;
      }
      
      long double i
                , j;

      Complex_t *z = malloc(sizeof(Complex_t));
      Complex_t *c = malloc(sizeof(Complex_t));

      i = map(x, 0.0, wi, xstt, xend);
      j = map(y, 0.0, he, ystt, yend);
      
      cc(0.0, 0.0, z);
      cc(  i,   j, c);
    
      long k;
      int added = 0;
      for (k = 0; k < it; ++k){
        squarec(z);
        addc(z, c);
        long delta = k * 100 / it;        
        if (getR(z) > 2.0){
          added = 1;
          pthread_mutex_lock( &mutex1 );
          sprintf(crd->buf + strlen(crd->buf), "{\"x\":%i,\"y\":%i,\"d\":%ld};", x, y, delta);
          pthread_mutex_unlock( &mutex1 );
          break ;
        }
      }
      if (!added){ 
          pthread_mutex_lock( &mutex1 );
          sprintf(crd->buf + strlen(crd->buf), "{\"x\":%i,\"y\":%i,\"d\":0};", x, y);
          pthread_mutex_unlock( &mutex1 );
      }
      
      free(z);
      free(c);

      //spiral drawing!
      //
      if (!top) {
        if (x < (wi - _x)) ++x;
        else top = 1;
      }else if (!right) {
        if (y < (he - _y)) ++y;
        else right = 1;
      }else if (!bottom) {
        if (x > _x) --x;
        else bottom = 1;
      }else if (!left) {
        if (y > _y) --y;
        else left = 1;
      }else{
        done = 1;
      }
    }
  }

  free(crd);
  busy--;
  pthread_mutex_lock( &mutex1 );
  sprintf(crd->buf + strlen(crd->buf), "f");
  pthread_mutex_unlock( &mutex1 );
  printf("%s\n", "thread has finished!"); 
  return NULL;
}
