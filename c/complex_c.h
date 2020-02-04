#include <math.h>
#include <gmp.h>


typedef struct ComplexNumbers{
  mpf_t *Re;
  mpf_t *Im;
} Complex;

int cc(mpf_t *re, mpf_t *im, Complex* z){
  z->Re = re;
  z->Im = im;
  return 0;
}

int squarec(Complex* z)
{
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

int checkR(Complex *z){
  mpf_t a, b, d; 
  mpf_init(a);
  mpf_init(b);
  mpf_init_set_str(d, "2", 10);

  mpf_mul(a, *z->Re, *z->Re);
  mpf_mul(b, *z->Im, *z->Im);

  mpf_add(a, a, b);
  mpf_sqrt(a, a); 

  return mpf_cmp(a, d);
}

int addc(Complex* a, Complex* b)
{
  mpf_add(*a->Re, *a->Re, *b->Re);
  mpf_add(*a->Im, *a->Im, *b->Im);
  return 0;
}
