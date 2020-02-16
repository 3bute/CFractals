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
  long double jx = 0;
  long double jy = 0;
  int julia = 0;

  if (!strcmp(crd->J, "true")) {
    julia = 1;
    jx = strtold(crd->Jx,  NULL);
    jy = strtold(crd->Jy, NULL);
  } else {
    julia = 0;
  }
    
  int right  = 1
    , longest = (wi > he) ? wi : he 
    , m = crd->idx * (longest / (2 * NUM_THREADS))  
    , mend = (crd->idx + 1) * (longest / (2 * NUM_THREADS))
    , left = 0
    , down = 0
    , x = 0
    , y = 0
    , up = 0
    , brk = 0
    , xs = wi / 2 
    , ys = he / 2 
    , s = 0;

  float n = 0.0;

  x = xs;
  y = ys;
  for (m; m < mend ; m++) {
    for (n = 0; n < 360; n+=0.5) {
        

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
        
        if (!julia) {
          cc(0.0, 0.0, z);
          cc(  i,   j, c);
        } else {
          cc(jx, jy, c);
          cc( i,  j, z);
        }
  
        long k;
        int added = 0;
        for (k = 0; k < it; ++k){
          squarec(z);
          addc(z, c);
          long delta = k * 100 / it;        
          if (getR(z) > crd->bound){
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

        x = (int) (xs + m * cos( (n * 3.141592625) / 180 )); 
        y = (int) (ys + m * sin( (n * 3.14) / 180 )); 
        printf("x: %i\n", x);
        printf("y: %i\n", y);
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
