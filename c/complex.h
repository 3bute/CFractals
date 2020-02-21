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

void *func1(void *args) {
  
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
    , x = wi * crd->idx / NUM_THREADS
    , xa = wi * (crd->idx + 1) / NUM_THREADS; 

  for (x; x < xa ; x++) {
    for (y = 0; y < he; y++) {
        
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
          if (getR(z) > crd->bound){
            added = 1;
            delta = k * 100 / it;        
            pthread_mutex_lock( &mutex1 );
            sprintf(crd->buf + strlen(crd->buf),
                "{\"x\":%i,\"y\":%i,\"d\":%ld};", x, y, delta);
            pthread_mutex_unlock( &mutex1 );
            break ;
          }
        }
        if (!added){ 
            pthread_mutex_lock( &mutex1 );
            sprintf(crd->buf + strlen(crd->buf),
                "{\"x\":%i,\"y\":%i,\"d\":0};", x, y);
            pthread_mutex_unlock( &mutex1 );
        }
        
        free(z);
        free(c);
    }
  }
  free(crd);
  busy--;
  pthread_mutex_lock( &mutex1 );
  sprintf(crd->buf + strlen(crd->buf), "f");
  pthread_mutex_unlock( &mutex1 );
  return NULL;
}

void *func2(void *args) {

  int prevType;
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &prevType);
  
  busy++;
  
  coords_t *crd = (coords_t *) args;
  
  long it   = strtol(crd->It, NULL, 10);
  int he    = strtol(crd->He, NULL, 10)
    , wi    = strtol(crd->Wi, NULL, 10) 
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
    
  int right   = 1
    , longest = (wi > he) ? wi : he 
    , m       = crd->idx * (longest / (2 * NUM_THREADS))  
    , mend    = (crd->idx + 1) * (longest / (2 * NUM_THREADS))
    , x       = 0
    , y       = 0
    , n       = 0
    , xs      = wi / 2 
    , ys      = he / 2;

  printf("m: %i, mend: %i, wi: %i, he: %i, xs: %i\n",
      m, mend, wi, he, xs);

  for (m; m < mend ; m++) {
   
    x = xs;
    y = ys - m;
    
    int left_down  = 1
      , right_down = 0
      , right_top  = 0
      , left_top   = 0;

    for (n = 0; n <= 4 * m; n++) {
       
      if (stop) {
        busy--;
        pthread_exit(0);
        return 0;
      }
      
      Complex_t *z = malloc(sizeof(Complex_t));
      Complex_t *c = malloc(sizeof(Complex_t));

      long double i, j;
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
        if (getR(z) > crd->bound){
          added = 1;
          delta = k * 100 / it;        
          pthread_mutex_lock( &mutex1 );
          sprintf(crd->buf + strlen(crd->buf),
              "{\"x\":%i,\"y\":%i,\"d\":%ld};", x, y, delta);
          pthread_mutex_unlock( &mutex1 );
          break ;
        }
      }

      if (!added){ 
          pthread_mutex_lock( &mutex1 );
          sprintf(crd->buf + strlen(crd->buf),
              "{\"x\":%i,\"y\":%i,\"d\":0};", x, y);
          pthread_mutex_unlock( &mutex1 );
      }
      
      free(z);
      free(c);

      if (left_down) {
        x--;
        y++;
        if (y >= (int)(he / 2)) {
          left_down = 0;
          right_down = 1;
        }
      } else  if (right_down) {
        x++;
        y++;
        if (x >= (int)(wi / 2)) {
          right_down = 0;
          right_top = 1;
        }
      } else if (right_top) {
        x++;
        y--;
        if (y <= (int)(he / 2)) {
          right_top = 0;
          left_top = 1;
        }
      } else if (left_top) {
        x--;
        y--;
        if (x <= (int)(wi / 2)) {
          left_top = 0;
          left_down = 1;
        }
      }
    }
  }
  busy--;
  free(crd);
  pthread_mutex_lock( &mutex1 );
  sprintf(crd->buf + strlen(crd->buf), "f");
  pthread_mutex_unlock( &mutex1 );
  return NULL;
}


