#include <gmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "complex_c.h"
#include <pthread.h>
#define NUM_THREADS 1 


void mapc(mpf_t *out, const char * a, const char * b, const char * c, const char * d, const char * e);

typedef struct coords {
   const char *Xstt;
   const char *Ystt;
   const char *Xend;
   const char *Yend;
   const char *Wi;
   const char *He;
   const char *It;
   char *buf;
} coords;


void *func(void *args) {
  struct coords *crd = (struct coords *) args;
  printf("%s\n", "thread has started!"); 
  
  long it = strtol(crd->It, NULL, 10);
  int he = strtol(crd->He, NULL, 10);
  int wi = strtol(crd->Wi, NULL, 10); 
  //int bound = strtol(Bound, NULL, 10); 
  //int zoom = strtol(Zoom, NULL, 10);
  
  //printf("He: %s\n", crd->He); 
  //printf("Wi: %s\n", crd->Wi); 
  //printf("It: %s\n", crd->It); 

  //printf("it: %ld\n", it);
  //printf("he: %i\n", he);
  //printf("wi: %d\n", wi);
  
  int x; 
  for (x = 0; x < wi; ++x){
    int y;
    for (y = 0; y < he; ++y){
     
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
          sprintf(crd->buf + strlen(crd->buf), "%ld;", delta);
          break ;
        }
      }
      if (!added){ 
        sprintf(crd->buf + strlen(crd->buf), "0;");
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
  printf("%s\n", "thread has finished!"); 
  return NULL;
}



void mapc(mpf_t *out, const char * a, const char * b, const char * c, const char * d, const char * e){
  
  mpf_t a0, b0, c0, d0, e0;
  mpf_t sub1, sub2, sub3, div1, mul1;
  mpf_set_default_prec(100);

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

void calcc(char *buf, const char *Xstt, const char *Ystt, const char *Xend, const char *Yend, const char *It, const char *Wi, const char *He, const char *Bound, const char *Zoom){
  
  
  pthread_t threads[NUM_THREADS];
  int rc
    , i;

  for( i = 0; i < NUM_THREADS; i++ ) {
      printf("starting thread n. %i\n", i);
      
      struct coords *crd = (struct coords *) malloc(sizeof(struct coords));
      
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
      char * wi = malloc(strlen(Wi) + 1);
      strcpy(wi, Wi);
      char * he = malloc(strlen(He) + 1);
      strcpy(he, He);
      char * it = malloc(strlen(It) + 1);
      strcpy(it, It);
      
      crd->Xstt = xstt;
      crd->Ystt = ystt;
      crd->Xend = xend;
      crd->Yend = yend;
      crd->Wi = wi;
      crd->He = he;
      crd->It = it;
      crd->buf = buf;

      rc = pthread_create(&threads[i], NULL, func, (void *)crd);
      if (rc) {
         printf("Error:unable to create thread, %d\n", rc);
         exit(-1);
      }
   }
   //pthread_exit(NULL);
}
