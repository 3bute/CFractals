void mapc(mpf_t *out, const char * a, const char * b, const char * c, const char * d, const char * e);

typedef struct coords {
   const char *Xstt;
   const char *Ystt;
   const char *Xend;
   const char *Yend;
   const char *Wi;
   const char *He;
   const char *It;
   int idx;
   char *buf;
} coords;

void mapc(mpf_t *out, const char * a, const char * b, const char * c, const char * d, const char * e){
  mpf_t a0, b0, c0, d0, e0;
  mpf_t sub1, sub2, sub3, div1, mul1;
  mpf_set_default_prec(200);

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

void *func(void *args) {
  int prevType;
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &prevType);
  
  struct coords *crd = (struct coords *) args;
  //in order to stop thread the cancel type should
  //be provided as async or deferred
  
  busy++;
  printf("%s\n", "thread has started!"); 
  long it = strtol(crd->It, NULL, 10);
  int he = strtol(crd->He, NULL, 10);
  int wi = strtol(crd->Wi, NULL, 10); 
  
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
      
      mpf_t i, j, z0, z1;
      mpf_init(i);
      mpf_init(j);
      mpf_init(z0);
      mpf_init(z1);
      mpf_set_str(z0, "0", 10);
      mpf_set_str(z1, "0", 10);

      
      char *str_x = malloc(sizeof(char)*4)
         , *str_y = malloc(sizeof(char)*4);
      
      Complex *z = malloc(sizeof(Complex));
      Complex *c = malloc(sizeof(Complex));

      sprintf(str_x, "%i", x);
      sprintf(str_y, "%i", y);

      mapc(&i, str_x, "0", crd->Wi, crd->Xstt, crd->Xend);
      mapc(&j, str_y, "0", crd->He, crd->Ystt, crd->Yend);
      
      cc(&z0, &z1, z);
      cc( &i,  &j, c);
    
      long k;
      int added = 0;
      for (k = 0; k < it; ++k){
        squarec(z);
        addc(z, c);
        long delta = k * 100 / it;        
        if (checkR(z)>0){
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
      free(str_y);
      free(str_x);
      mpf_clear(z0);
      mpf_clear(z1);
      mpf_clear(j);
      mpf_clear(i);

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

