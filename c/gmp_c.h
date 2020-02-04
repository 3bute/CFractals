#include <gmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "complex_c.h"



void mapc(mpf_t *out, const char * a, const char * b, const char * c, const char * d, const char * e){
  mpf_t a0, b0, c0, d0, e0;
  mpf_t sub1, sub2, sub3, div1, mul1;

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
  
  mpf_set_str(a0, a, 10);
  mpf_set_str(b0, b, 10);
  mpf_set_str(c0, c, 10);
  mpf_set_str(d0, d, 10);
  mpf_set_str(e0, e, 10);
  
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

char *calcc(const char *Xstt, const char *Ystt, const char *Xend, const char *Yend, const char *It, const char *Wi, const char *He, const char *Bound, const char *Zoom){
  
  int it = strtol(It, NULL, 10);
  int he = strtol(He, NULL, 10);
  int wi = strtol(Wi, NULL, 10); 
  int bound = strtol(Bound, NULL, 10); 
  int zoom = strtol(Zoom, NULL, 10);

  char *result = malloc(wi*he*sizeof(char));

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
      
      char *str_x = malloc(sizeof(char)*3)
         , *str_y = malloc(sizeof(char)*3);
      
      Complex *z = malloc(sizeof(Complex));
      Complex *c = malloc(sizeof(Complex));

      sprintf(str_x, "%i", x);
      sprintf(str_y, "%i", y);

      mapc(&i, str_x, "0", Wi, Xstt, Xend);
      mapc(&j, str_y, "0", He, Ystt, Yend);
     
      cc(&z0, &z1, z);
      cc( &i,  &j, c);
      
      int k;
      int added = 0;
      
      for (k = 0; k < it; ++k){
        
        squarec(z);
        addc(z, c);
        
        int delta = k * 100 / it;        
        if (checkR(z)>0){
          char temp[3];
          sprintf(temp, "%i", delta);
          strncat(result, temp, 3);
          strncat(result, ";", 1);
          added = 1;
          //free(temp);
          break ;
        }
      }
      if (!added) strncat(result, "0;", 2); 
      
      free(z);
      free(c);
      free(str_x);
      free(str_y);
      mpf_clear(z0);
      mpf_clear(z1);
      mpf_clear(j);
      mpf_clear(i);
    }
  }
  return result;
}
