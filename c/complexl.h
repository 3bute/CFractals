#include <math.h>
#include <time.h>
#include <gmp.h>

extern volatile int busy;
extern volatile int stop;
extern pthread_mutex_t mutex1;

typedef struct {
  mpf_t *Re;
  mpf_t *Im;
} Complexl_t;

int ccl(mpf_t *re, mpf_t *im, Complexl_t* z){
  z->Re = re;
  z->Im = im;
  return 0;
}

int squarecl(Complexl_t* z) {
  mpf_t a, b, c;
  mpf_init(a);
  mpf_init(b);
  mpf_init(c);

  mpf_set(c, *z->Im);

  mpf_mul(a, *z->Re, *z->Im);
  mpf_add(*z->Im, a, a); 

  mpf_mul(b, *z->Re, *z->Re);
  mpf_mul(c, c, c);
  mpf_sub(*z->Re, b, c);
  
  mpf_clear(a);
  mpf_clear(b);
  mpf_clear(c);

  return 0;
}

int checkR(Complexl_t *z, double bound) {
  mpf_t a, b, c; 
  mpf_init(a);
  mpf_init(b);
  mpf_init(c);
  mpf_set_d(c, bound);
  mpf_mul(a, *z->Re, *z->Re);
  mpf_mul(b, *z->Im, *z->Im);

  mpf_add(a, a, b);
  mpf_sqrt(a, a); 
  
  //very imp
  mpf_clear(a);
  mpf_clear(b);
  mpf_clear(c);

  return mpf_cmp(a, c);
}

int addcl(Complexl_t* a, Complexl_t* b) {
  mpf_add(*a->Re, *a->Re, *b->Re);
  mpf_add(*a->Im, *a->Im, *b->Im);
  return 0;
}

void mapl(mpf_t *out, const char * a, const char * b, const char * c, const char * d, const char * e) {

  mpf_t a0, b0, c0, d0, e0;
  mpf_t sub1, sub2, sub3, div1, mul1;
  mpf_set_default_prec( 5 * strlen(d) );

  mpf_init(a0);
  mpf_init(b0);
  mpf_init(c0);
  mpf_init(d0);
  mpf_init(e0);

  mpf_init(sub1);
  mpf_init(sub2);
  mpf_init(sub3);
  mpf_init(div1);
  mpf_init(mul1);
  
  mpf_set_str(a0, a, 0);
  mpf_set_str(b0, b, 0);
  mpf_set_str(c0, c, 0);
  mpf_set_str(d0, d, 0);
  mpf_set_str(e0, e, 0);
  
  mpf_sub(sub1, a0, b0);
  mpf_sub(sub2, c0, b0);
  mpf_sub(sub3, e0, d0);
  mpf_div(div1, sub1, sub2);
  mpf_mul(mul1, div1, sub3);
  mpf_add(*out, mul1, d0);
  

  mpf_clear(a0);
  mpf_clear(b0);
  mpf_clear(c0);
  mpf_clear(d0);
  mpf_clear(e0);
  mpf_clear(sub1);
  mpf_clear(sub2);
  mpf_clear(sub3);
  mpf_clear(div1);
  mpf_clear(mul1);

}

void *funcl1(void *args) {

  int prevType;
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &prevType);
  coords_t *crd = (coords_t *) args;
  //in order to stop thread the cancel type should
  //be provided as async or deferred
  
  busy++;
  long it = strtol(crd->It, NULL, 10);
  int he = strtol(crd->He, NULL, 10);
  int wi = strtol(crd->Wi, NULL, 10); 
  
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
      
      mpf_t i, j, z0, z1;
      mpf_init(i);
      mpf_init(j);
      mpf_init(z0);
      mpf_init(z1);
      
      char *str_x = malloc(sizeof(char)*4)
         , *str_y = malloc(sizeof(char)*4);
      
      Complexl_t *z = malloc(sizeof(Complexl_t));
      Complexl_t *c = malloc(sizeof(Complexl_t));

      sprintf(str_x, "%i", x);
      sprintf(str_y, "%i", y);

      mapl(&i, str_x, "0", crd->Wi, crd->Xstt, crd->Xend);
      mapl(&j, str_y, "0", crd->He, crd->Ystt, crd->Yend);
      
      if (!strcmp(crd->J, "true")) {
        mpf_set_str(z0, crd->Jx, 10);
        mpf_set_str(z1, crd->Jy, 10);
        ccl(&z0, &z1, c);
        ccl( &i,  &j, z);
      } else {
        mpf_set_str(z0, "0", 10);
        mpf_set_str(z1, "0", 10);
        ccl(&z0, &z1, z);
        ccl( &i,  &j, c);
      }
    
      long k;
      int added = 0;
      for (k = 0; k < it; ++k){
        squarecl(z);
        addcl(z, c);
        if (checkR(z, crd->bound)>0){
          added = 1;
          pthread_mutex_lock( &mutex1 );
          long delta = k * 100 / it;        
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
      free(str_y);
      free(str_x);
      mpf_clear(z0);
      mpf_clear(z1);
      mpf_clear(j);
      mpf_clear(i);
    }
  }
  busy--;
  free(crd);
  pthread_mutex_lock( &mutex1 );
  sprintf(crd->buf + strlen(crd->buf), "f");
  pthread_mutex_unlock( &mutex1 );
  return NULL;

}

void *funcl2(void *args) {

  int prevType;
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &prevType);
  
  coords_t *crd = (coords_t *) args;
  //in order to stop thread the cancel type should
  //be provided as async or deferred
  
  busy++;
  long it = strtol(crd->It, NULL, 10);
  int he = strtol(crd->He, NULL, 10);
  int wi = strtol(crd->Wi, NULL, 10); 
  
  int right   = 1
    , longest = (wi > he) ? wi : he 
    , m       = crd->idx * (longest / (2 * NUM_THREADS))  
    , mend    = (crd->idx + 1) * (longest / (2 * NUM_THREADS))
    , x       = 0
    , y       = 0
    , n       = 0
    , xs      = wi / 2 
    , ys      = he / 2;

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

      
      mpf_t i, j, z0, z1;
      mpf_init(i);
      mpf_init(j);
      mpf_init(z0);
      mpf_init(z1);
      
      char *str_x = malloc(sizeof(char)*4)
         , *str_y = malloc(sizeof(char)*4);
      
      Complexl_t *z = malloc(sizeof(Complexl_t));
      Complexl_t *c = malloc(sizeof(Complexl_t));

      sprintf(str_x, "%i", x);
      sprintf(str_y, "%i", y);

      mapl(&i, str_x, "0", crd->Wi, crd->Xstt, crd->Xend);
      mapl(&j, str_y, "0", crd->He, crd->Ystt, crd->Yend);
      
      if (!strcmp(crd->J, "true")) {
        mpf_set_str(z0, crd->Jx, 10);
        mpf_set_str(z1, crd->Jy, 10);
        ccl(&z0, &z1, c);
        ccl( &i,  &j, z);
      } else {
        mpf_set_str(z0, "0", 10);
        mpf_set_str(z1, "0", 10);
        ccl(&z0, &z1, z);
        ccl( &i,  &j, c);
      }
    
      long k;
      int added = 0;
      for (k = 0; k < it; ++k){
        squarecl(z);
        addcl(z, c);
        if (checkR(z, crd->bound)>0){
          added = 1;
          long delta = k * 100 / it;        
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
      
      free(z);
      free(c);
      free(str_y);
      free(str_x);
      mpf_clear(z0);
      mpf_clear(z1);
      mpf_clear(j);
      mpf_clear(i);
    }
  }
  busy--;
  free(crd);
  pthread_mutex_lock( &mutex1 );
  sprintf(crd->buf + strlen(crd->buf), "f");
  pthread_mutex_unlock( &mutex1 );
  return NULL;

}
