#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "raytracer-project2.h"

double restrict01(double x, char *var_name)
{
  if (x<0) {
    fprintf(stderr,"warning: %s<0 (%lf), setting to 0\n",var_name,x);
    return 0.0;
  } else if (x>1) {
    fprintf(stderr,"warning: %s>1 (%lf), setting to 1\n",var_name,x);
    return 1.0;
  } else 
    return x;
}

color *color_new(double r, double g, double b)
{
  color *c = (color*)malloc(sizeof(color));
  check_malloc("color_new",c);
  c->r = restrict01(r,"r");
  c->g = restrict01(g,"g");
  c->b = restrict01(b,"b");
  return c;
}

char *color_format = "(r=%lf,g=%lf,b=%lf)";

char *color_tos(color *c)
{
  char buf[256];
  sprintf(buf,color_format,c->r,c->g,c->b);
  return strdup(buf);
}

void color_show(FILE *f, color *c)
{
  fprintf(f,color_format,c->r,c->g,c->b);
}

color   *color_add(color *c1, color *c2)
{
  double r_new = c1->r + c2->r;
  double g_new = c1->g + c2->g;
  double b_new = c1->b + c2->b;
  if (r_new > 1){
    r_new = 1.0;
  }
  if (g_new > 1){
    g_new = 1.0;
  }
  if (b_new > 1){
    b_new = 1.0;
  }
  return color_new(r_new, g_new, b_new);
}

color   *color_modulate(color *c1, color *c2)
{
  double r_new = c1->r * c2->r;
  double g_new = c1->g * c2->g;
  double b_new = c1->b * c2->b;
  return color_new(r_new, g_new, b_new);
}

color   *color_scale(double scalar, color *c)
{
  double r_new = scalar * c->r;
  double g_new = scalar * c->g;
  double b_new = scalar * c->b;
  if (r_new > 1){
    r_new = 1.0;
  }
  if (g_new > 1){
    g_new = 1.0;
  }
  if (b_new > 1){
    b_new = 1.0;
  }
  return color_new(r_new, g_new, b_new);
}